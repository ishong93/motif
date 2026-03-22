#include <motif/clipboard/Transfer.h>
#include <motif/core/Application.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <cstring>
#include <algorithm>

namespace motif {

// ── SelectionAtoms ───────────────────────────────────────────────────

SelectionAtoms& SelectionAtoms::instance() {
    static SelectionAtoms atoms;
    return atoms;
}

void SelectionAtoms::initialize(Display* dpy) {
    if (!dpy) return;
    clipboard    = XInternAtom(dpy, "CLIPBOARD", False);
    targets      = XInternAtom(dpy, "TARGETS", False);
    multiple     = XInternAtom(dpy, "MULTIPLE", False);
    timestamp    = XInternAtom(dpy, "TIMESTAMP", False);
    utf8String   = XInternAtom(dpy, "UTF8_STRING", False);
    compoundText = XInternAtom(dpy, "COMPOUND_TEXT", False);
    textPlain    = XInternAtom(dpy, "text/plain", False);
    motifDrop    = XInternAtom(dpy, "_MOTIF_DROP", False);
}

// ── TransferManager ──────────────────────────────────────────────────

TransferManager& TransferManager::instance() {
    static TransferManager mgr;
    return mgr;
}

void TransferManager::initialize(Display* dpy) {
    display_ = dpy;
    SelectionAtoms::instance().initialize(dpy);
}

// ── Clipboard high-level API ─────────────────────────────────────────

bool TransferManager::copyText(Window owner, const std::string& text) {
    if (!display_) return false;

    auto& atoms = SelectionAtoms::instance();
    clipboardText_ = text;

    SelectionOffer offer;
    offer.selection = atoms.clipboard;
    offer.owner = owner;
    offer.supportedTargets = {atoms.targets, atoms.utf8String, XA_STRING, atoms.textPlain};
    offer.convertCb = [this, &atoms](Atom target, std::vector<uint8_t>& dataOut, Atom& typeOut) -> bool {
        if (target == atoms.targets) {
            // Return list of supported targets
            std::vector<Atom> tgts = {atoms.targets, atoms.utf8String, XA_STRING, atoms.textPlain};
            dataOut.resize(tgts.size() * sizeof(Atom));
            std::memcpy(dataOut.data(), tgts.data(), dataOut.size());
            typeOut = XA_ATOM;
            return true;
        }
        if (target == atoms.utf8String || target == XA_STRING || target == atoms.textPlain) {
            dataOut.assign(clipboardText_.begin(), clipboardText_.end());
            typeOut = (target == atoms.utf8String) ? atoms.utf8String : XA_STRING;
            return true;
        }
        return false;
    };

    return ownSelection(offer);
}

void TransferManager::pasteText(Window requestor,
                                 std::function<void(const std::string&)> callback) {
    auto& atoms = SelectionAtoms::instance();

    // If we own the clipboard, short-circuit
    if (isOwner(atoms.clipboard)) {
        if (callback) callback(clipboardText_);
        return;
    }

    // Request UTF8_STRING from clipboard owner
    requestBestText(atoms.clipboard, requestor, std::move(callback));
}

// ── Selection ownership ──────────────────────────────────────────────

bool TransferManager::ownSelection(const SelectionOffer& offer) {
    if (!display_) return false;

    XSetSelectionOwner(display_, offer.selection, offer.owner, CurrentTime);
    if (XGetSelectionOwner(display_, offer.selection) != offer.owner) {
        return false;
    }

    ownedSelections_[offer.selection] = offer;
    return true;
}

void TransferManager::disownSelection(Atom selection) {
    if (display_) {
        XSetSelectionOwner(display_, selection, None, CurrentTime);
    }
    ownedSelections_.erase(selection);
}

bool TransferManager::isOwner(Atom selection) const {
    return ownedSelections_.find(selection) != ownedSelections_.end();
}

// ── Selection requests ───────────────────────────────────────────────

uint64_t TransferManager::requestSelection(Atom selection, Atom target,
                                            Window requestor,
                                            TransferRequest::Callback cb) {
    if (!display_) return 0;

    uint64_t id = nextRequestId_++;

    // Use a unique property name for this request
    std::string propName = "_MOTIF_TRANSFER_" + std::to_string(id);
    Atom property = XInternAtom(display_, propName.c_str(), False);

    TransferRequest req;
    req.selection = selection;
    req.target = target;
    req.property = property;
    req.requestor = requestor;
    req.callback = std::move(cb);
    req.id = id;

    pendingRequests_[property] = std::move(req);

    XConvertSelection(display_, selection, target, property, requestor, CurrentTime);
    return id;
}

void TransferManager::requestBestText(Atom selection, Window requestor,
                                       std::function<void(const std::string&)> cb) {
    auto& atoms = SelectionAtoms::instance();

    // First request TARGETS to see what's available
    requestSelection(selection, atoms.targets, requestor,
        [this, selection, requestor, cb, &atoms]
        (const std::string& /*type*/, const uint8_t* data, size_t size, bool success) {
            Atom bestTarget = XA_STRING; // fallback

            if (success && data && size >= sizeof(Atom)) {
                size_t count = size / sizeof(Atom);
                const Atom* targetList = reinterpret_cast<const Atom*>(data);

                // Prefer UTF8_STRING > STRING > text/plain
                for (size_t i = 0; i < count; ++i) {
                    if (targetList[i] == atoms.utf8String) {
                        bestTarget = atoms.utf8String;
                        break;
                    }
                    if (targetList[i] == XA_STRING) bestTarget = XA_STRING;
                    if (targetList[i] == atoms.textPlain && bestTarget == XA_STRING) {
                        bestTarget = atoms.textPlain;
                    }
                }
            }

            // Now request the actual data
            requestSelection(selection, bestTarget, requestor,
                [cb](const std::string& /*type*/, const uint8_t* d, size_t sz, bool ok) {
                    if (ok && d && sz > 0) {
                        cb(std::string(reinterpret_cast<const char*>(d), sz));
                    } else {
                        cb("");
                    }
                });
        });
}

// ── Event handlers ───────────────────────────────────────────────────

void TransferManager::handleSelectionRequest(const XSelectionRequestEvent& ev) {
    // Someone is asking us for data
    XSelectionEvent reply;
    reply.type = SelectionNotify;
    reply.display = ev.display;
    reply.requestor = ev.requestor;
    reply.selection = ev.selection;
    reply.target = ev.target;
    reply.time = ev.time;
    reply.property = None; // fail by default

    auto it = ownedSelections_.find(ev.selection);
    if (it != ownedSelections_.end() && it->second.convertCb) {
        std::vector<uint8_t> data;
        Atom typeOut = XA_STRING;

        if (it->second.convertCb(ev.target, data, typeOut)) {
            Atom property = ev.property != None ? ev.property : ev.target;

            XChangeProperty(display_, ev.requestor, property,
                           typeOut,
                           (typeOut == XA_ATOM) ? 32 : 8,
                           PropModeReplace,
                           data.data(),
                           (typeOut == XA_ATOM)
                               ? static_cast<int>(data.size() / sizeof(Atom))
                               : static_cast<int>(data.size()));

            reply.property = property;
        }
    }

    XSendEvent(display_, ev.requestor, False, 0, reinterpret_cast<XEvent*>(&reply));
    XFlush(display_);
}

void TransferManager::handleSelectionNotify(const XSelectionEvent& ev) {
    // We received data we asked for
    auto it = pendingRequests_.find(ev.property);
    if (it == pendingRequests_.end()) return;

    TransferRequest req = std::move(it->second);
    pendingRequests_.erase(it);

    if (ev.property == None) {
        // Transfer failed
        if (req.callback) req.callback("", nullptr, 0, false);
        return;
    }

    // Read the property data
    Atom actualType;
    int actualFormat;
    unsigned long nitems, bytesAfter;
    unsigned char* data = nullptr;

    XGetWindowProperty(display_, req.requestor, ev.property,
                       0, 1024 * 1024, True, AnyPropertyType,
                       &actualType, &actualFormat,
                       &nitems, &bytesAfter, &data);

    if (data) {
        size_t dataSize = nitems * (actualFormat / 8);
        char* atomName = XGetAtomName(display_, actualType);
        std::string typeName = atomName ? atomName : "";
        if (atomName) XFree(atomName);
        if (req.callback) {
            req.callback(typeName, data, dataSize, true);
        }
        XFree(data);
    } else {
        if (req.callback) req.callback("", nullptr, 0, false);
    }

    XDeleteProperty(display_, req.requestor, ev.property);
}

void TransferManager::handleSelectionClear(const XSelectionClearEvent& ev) {
    auto it = ownedSelections_.find(ev.selection);
    if (it != ownedSelections_.end()) {
        if (it->second.lostCb) it->second.lostCb();
        ownedSelections_.erase(it);
    }
}

} // namespace motif
