#include <motif/dnd/DragContext.h>
#include <motif/dnd/DragIcon.h>
#include <motif/dnd/DropSiteManager.h>
#include <motif/core/Application.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <cmath>
#include <cstring>

namespace motif {

// ── XdndAtoms ────────────────────────────────────────────────────────

XdndAtoms& XdndAtoms::instance() {
    static XdndAtoms atoms;
    return atoms;
}

void XdndAtoms::initialize(Display* dpy) {
    if (!dpy) return;
    XdndAware      = XInternAtom(dpy, "XdndAware", False);
    XdndEnter      = XInternAtom(dpy, "XdndEnter", False);
    XdndPosition   = XInternAtom(dpy, "XdndPosition", False);
    XdndStatus     = XInternAtom(dpy, "XdndStatus", False);
    XdndLeave      = XInternAtom(dpy, "XdndLeave", False);
    XdndDrop       = XInternAtom(dpy, "XdndDrop", False);
    XdndFinished   = XInternAtom(dpy, "XdndFinished", False);
    XdndSelection  = XInternAtom(dpy, "XdndSelection", False);
    XdndTypeList   = XInternAtom(dpy, "XdndTypeList", False);
    XdndActionCopy = XInternAtom(dpy, "XdndActionCopy", False);
    XdndActionMove = XInternAtom(dpy, "XdndActionMove", False);
    XdndActionLink = XInternAtom(dpy, "XdndActionLink", False);
    XdndProxy      = XInternAtom(dpy, "XdndProxy", False);
}

// ── DragContext ──────────────────────────────────────────────────────

DragContext::DragContext(Widget* source, const std::vector<std::string>& exportTargets)
    : source_(source)
    , exportTargets_(exportTargets)
{
    auto* dpy = Application::instance().display();
    if (dpy) {
        XdndAtoms::instance().initialize(dpy);
        // Cache Atom versions of target type strings
        for (const auto& t : exportTargets_) {
            exportAtoms_.push_back(XInternAtom(dpy, t.c_str(), False));
        }
    }
}

DragContext::~DragContext() {
    if (dragging_) cancelDrag();
}

void DragContext::startDrag(int rootX, int rootY) {
    if (dragging_) return;

    dragging_ = true;
    thresholdMet_ = false;
    startX_ = rootX;
    startY_ = rootY;

    auto& app = Application::instance();
    auto* dpy = app.display();
    if (!dpy || !source_ || !source_->window()) return;

    // Grab pointer
    Cursor cursor = XCreateFontCursor(dpy, XC_hand2);
    XGrabPointer(dpy, source_->window(), False,
                 ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync,
                 None, cursor, CurrentTime);
    XFreeCursor(dpy, cursor);

    // Own XdndSelection so targets can request data
    auto& atoms = XdndAtoms::instance();
    XSetSelectionOwner(dpy, atoms.XdndSelection, source_->window(), CurrentTime);
}

void DragContext::cancelDrag() {
    if (!dragging_) return;
    dragging_ = false;

    auto* dpy = Application::instance().display();
    if (dpy) {
        XUngrabPointer(dpy, CurrentTime);
        if (xdndTarget_ != None) {
            sendXdndLeave(xdndTarget_);
            xdndTarget_ = None;
        }
    }

    if (dragIcon_) dragIcon_->hide();
    if (finishCb_) finishCb_(action_, false);
}

void DragContext::handleMotion(int rootX, int rootY) {
    if (!dragging_) return;

    // Check threshold
    if (!thresholdMet_) {
        int dx = rootX - startX_;
        int dy = rootY - startY_;
        if (std::abs(dx) < threshold_ && std::abs(dy) < threshold_) return;
        thresholdMet_ = true;
        if (dragIcon_) dragIcon_->show(rootX, rootY);
    }

    if (dragIcon_) dragIcon_->moveTo(rootX, rootY);

    // Try internal drop sites first
    DropSiteManager::instance().handleDragMotion(*this, rootX, rootY);

    // Then try XDND for external windows
    auto* dpy = Application::instance().display();
    if (!dpy) return;

    Window newTarget = findXdndAwareWindow(rootX, rootY);

    if (newTarget != xdndTarget_) {
        // Left old XDND target
        if (xdndTarget_ != None) {
            sendXdndLeave(xdndTarget_);
        }

        xdndTarget_ = newTarget;
        xdndAccepted_ = false;

        // Entered new XDND target
        if (xdndTarget_ != None) {
            protocol_ = Protocol::Xdnd;
            sendXdndEnter(xdndTarget_);
        } else {
            protocol_ = Protocol::Internal;
        }
    }

    if (xdndTarget_ != None) {
        sendXdndPosition(xdndTarget_, rootX, rootY);
    }
}

void DragContext::handleDrop(int rootX, int rootY) {
    if (!dragging_) return;
    dragging_ = false;

    auto* dpy = Application::instance().display();
    if (dpy) XUngrabPointer(dpy, CurrentTime);
    if (dragIcon_) dragIcon_->hide();

    if (protocol_ == Protocol::Xdnd && xdndTarget_ != None) {
        // XDND drop: send XdndDrop message
        sendXdndDrop(xdndTarget_);
        // Finish callback will fire when XdndFinished arrives
        return;
    }

    // Internal drop
    bool success = DropSiteManager::instance().handleDrop(*this, rootX, rootY);
    xdndTarget_ = None;
    if (finishCb_) finishCb_(action_, success);
}

bool DragContext::convertData(const std::string& targetType, std::string& dataOut) const {
    if (convertCb_) return convertCb_(targetType, dataOut);
    return false;
}

// ── XDND message senders ─────────────────────────────────────────────

void DragContext::sendXdndEnter(Window target) {
    auto* dpy = Application::instance().display();
    auto& atoms = XdndAtoms::instance();
    if (!dpy || !source_) return;

    XClientMessageEvent msg = {};
    msg.type = ClientMessage;
    msg.display = dpy;
    msg.window = target;
    msg.message_type = atoms.XdndEnter;
    msg.format = 32;
    msg.data.l[0] = static_cast<long>(source_->window());
    msg.data.l[1] = (5 << 24); // XDND version 5

    // If > 3 types, set bit 0 of data.l[1] and put full list in XdndTypeList
    if (exportAtoms_.size() > 3) {
        msg.data.l[1] |= 1;
        XChangeProperty(dpy, source_->window(), atoms.XdndTypeList, XA_ATOM, 32,
                        PropModeReplace,
                        reinterpret_cast<const unsigned char*>(exportAtoms_.data()),
                        static_cast<int>(exportAtoms_.size()));
    }

    // Up to 3 types inline
    for (size_t i = 0; i < 3 && i < exportAtoms_.size(); ++i) {
        msg.data.l[2 + i] = static_cast<long>(exportAtoms_[i]);
    }

    XSendEvent(dpy, target, False, NoEventMask, reinterpret_cast<XEvent*>(&msg));
}

void DragContext::sendXdndPosition(Window target, int rootX, int rootY) {
    auto* dpy = Application::instance().display();
    auto& atoms = XdndAtoms::instance();
    if (!dpy || !source_) return;

    XClientMessageEvent msg = {};
    msg.type = ClientMessage;
    msg.display = dpy;
    msg.window = target;
    msg.message_type = atoms.XdndPosition;
    msg.format = 32;
    msg.data.l[0] = static_cast<long>(source_->window());
    msg.data.l[1] = 0; // reserved
    msg.data.l[2] = (rootX << 16) | (rootY & 0xFFFF);
    msg.data.l[3] = CurrentTime;
    msg.data.l[4] = static_cast<long>(actionToXdndAtom());

    XSendEvent(dpy, target, False, NoEventMask, reinterpret_cast<XEvent*>(&msg));
}

void DragContext::sendXdndLeave(Window target) {
    auto* dpy = Application::instance().display();
    auto& atoms = XdndAtoms::instance();
    if (!dpy || !source_) return;

    XClientMessageEvent msg = {};
    msg.type = ClientMessage;
    msg.display = dpy;
    msg.window = target;
    msg.message_type = atoms.XdndLeave;
    msg.format = 32;
    msg.data.l[0] = static_cast<long>(source_->window());

    XSendEvent(dpy, target, False, NoEventMask, reinterpret_cast<XEvent*>(&msg));
}

void DragContext::sendXdndDrop(Window target) {
    auto* dpy = Application::instance().display();
    auto& atoms = XdndAtoms::instance();
    if (!dpy || !source_) return;

    XClientMessageEvent msg = {};
    msg.type = ClientMessage;
    msg.display = dpy;
    msg.window = target;
    msg.message_type = atoms.XdndDrop;
    msg.format = 32;
    msg.data.l[0] = static_cast<long>(source_->window());
    msg.data.l[2] = CurrentTime;

    XSendEvent(dpy, target, False, NoEventMask, reinterpret_cast<XEvent*>(&msg));
}

Window DragContext::findXdndAwareWindow(int rootX, int rootY) const {
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();
    auto& atoms = XdndAtoms::instance();
    if (!dpy) return None;

    Window root = app.rootWindow();
    Window child = None;
    int destX, destY;

    // Walk down the window tree at (rootX, rootY)
    Window current = root;
    while (true) {
        if (!XTranslateCoordinates(dpy, root, current, rootX, rootY,
                                    &destX, &destY, &child)) break;
        if (child == None) break;

        // Check if child has XdndAware property
        Atom actualType;
        int actualFormat;
        unsigned long nitems, bytesAfter;
        unsigned char* data = nullptr;

        XGetWindowProperty(dpy, child, atoms.XdndAware,
                           0, 1, False, XA_ATOM,
                           &actualType, &actualFormat,
                           &nitems, &bytesAfter, &data);

        if (actualType == XA_ATOM && nitems > 0 && data) {
            Atom version = *reinterpret_cast<Atom*>(data);
            XFree(data);
            if (version >= 3) return child; // XDND v3+
        }
        if (data) XFree(data);

        current = child;
    }

    return None;
}

Atom DragContext::actionToXdndAtom() const {
    auto& atoms = XdndAtoms::instance();
    switch (action_) {
        case DragAction::Copy: return atoms.XdndActionCopy;
        case DragAction::Move: return atoms.XdndActionMove;
        case DragAction::Link: return atoms.XdndActionLink;
    }
    return atoms.XdndActionCopy;
}

// ── XDND event handlers ─────────────────────────────────────────────

void DragContext::handleXdndStatus(const XClientMessageEvent& ev) {
    xdndAccepted_ = (ev.data.l[1] & 1) != 0;
}

void DragContext::handleXdndFinished(const XClientMessageEvent& ev) {
    bool accepted = (ev.data.l[1] & 1) != 0;
    xdndTarget_ = None;
    if (finishCb_) finishCb_(action_, accepted);
}

bool DragContext::handleSelectionRequest(const XSelectionRequestEvent& ev) {
    auto& atoms = XdndAtoms::instance();
    if (ev.selection != atoms.XdndSelection) return false;

    auto* dpy = Application::instance().display();
    if (!dpy) return false;

    // Convert the requested target
    char* targetName = XGetAtomName(dpy, ev.target);
    std::string targetStr = targetName ? targetName : "";
    if (targetName) XFree(targetName);

    std::string data;
    bool success = convertData(targetStr, data);

    XSelectionEvent reply = {};
    reply.type = SelectionNotify;
    reply.display = dpy;
    reply.requestor = ev.requestor;
    reply.selection = ev.selection;
    reply.target = ev.target;
    reply.time = ev.time;
    reply.property = None;

    if (success) {
        Atom property = ev.property != None ? ev.property : ev.target;
        XChangeProperty(dpy, ev.requestor, property,
                        ev.target, 8, PropModeReplace,
                        reinterpret_cast<const unsigned char*>(data.data()),
                        static_cast<int>(data.size()));
        reply.property = property;
    }

    XSendEvent(dpy, ev.requestor, False, 0, reinterpret_cast<XEvent*>(&reply));
    return true;
}

} // namespace motif
