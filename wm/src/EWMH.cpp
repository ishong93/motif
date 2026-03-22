#include <motif/wm/EWMH.h>
#include <motif/wm/WindowManager.h>

#include <X11/Xatom.h>
#include <cstring>
#include <algorithm>

namespace motif::wm {

EWMH::EWMH(WindowManager& wm) : wm_(wm) {}

void EWMH::initialize() {
    auto* dpy = wm_.display();

    // Intern all EWMH atoms
    netSupported_               = XInternAtom(dpy, "_NET_SUPPORTED", False);
    netClientList_              = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
    netClientListStacking_      = XInternAtom(dpy, "_NET_CLIENT_LIST_STACKING", False);
    netActiveWindow_            = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    netWmName_                  = XInternAtom(dpy, "_NET_WM_NAME", False);
    netWmDesktop_               = XInternAtom(dpy, "_NET_WM_DESKTOP", False);
    netWmState_                 = XInternAtom(dpy, "_NET_WM_STATE", False);
    netWmStateFullscreen_       = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
    netWmStateMaximizedVert_    = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    netWmStateMaximizedHorz_    = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    netWmStateHidden_           = XInternAtom(dpy, "_NET_WM_STATE_HIDDEN", False);
    netWmStateAbove_            = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", False);
    netWmWindowType_            = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    netWmWindowTypeNormal_      = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NORMAL", False);
    netWmWindowTypeDialog_      = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    netWmWindowTypeDock_        = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
    netWmWindowTypeToolbar_     = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
    netWmWindowTypeMenu_        = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_MENU", False);
    netWmWindowTypeUtility_     = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_UTILITY", False);
    netWmWindowTypeSplash_      = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_SPLASH", False);
    netWmWindowTypeDesktop_     = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
    netWmAllowedActions_        = XInternAtom(dpy, "_NET_WM_ALLOWED_ACTIONS", False);
    netWmActionMove_            = XInternAtom(dpy, "_NET_WM_ACTION_MOVE", False);
    netWmActionResize_          = XInternAtom(dpy, "_NET_WM_ACTION_RESIZE", False);
    netWmActionMinimize_        = XInternAtom(dpy, "_NET_WM_ACTION_MINIMIZE", False);
    netWmActionMaximizeHorz_    = XInternAtom(dpy, "_NET_WM_ACTION_MAXIMIZE_HORZ", False);
    netWmActionMaximizeVert_    = XInternAtom(dpy, "_NET_WM_ACTION_MAXIMIZE_VERT", False);
    netWmActionClose_           = XInternAtom(dpy, "_NET_WM_ACTION_CLOSE", False);
    netWmActionFullscreen_      = XInternAtom(dpy, "_NET_WM_ACTION_FULLSCREEN", False);
    netCloseWindow_             = XInternAtom(dpy, "_NET_CLOSE_WINDOW", False);
    netWmStrut_                 = XInternAtom(dpy, "_NET_WM_STRUT", False);
    netWmStrutPartial_          = XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL", False);
    netWorkarea_                = XInternAtom(dpy, "_NET_WORKAREA", False);
    netSupportingWmCheck_       = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
    netNumberOfDesktops_        = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False);
    netCurrentDesktop_          = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);
    utf8String_                 = XInternAtom(dpy, "UTF8_STRING", False);

    setRootProperties();
}

void EWMH::setRootProperties() {
    auto* dpy = wm_.display();
    auto root = wm_.root();

    // Create a small check window for _NET_SUPPORTING_WM_CHECK
    wmCheckWin_ = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);

    // Set _NET_SUPPORTING_WM_CHECK on root and check window
    XChangeProperty(dpy, root, netSupportingWmCheck_, XA_WINDOW, 32,
                    PropModeReplace, reinterpret_cast<unsigned char*>(&wmCheckWin_), 1);
    XChangeProperty(dpy, wmCheckWin_, netSupportingWmCheck_, XA_WINDOW, 32,
                    PropModeReplace, reinterpret_cast<unsigned char*>(&wmCheckWin_), 1);

    // Set _NET_WM_NAME on check window
    const char* wmName = "motif-wm";
    XChangeProperty(dpy, wmCheckWin_, netWmName_, utf8String_, 8,
                    PropModeReplace,
                    reinterpret_cast<const unsigned char*>(wmName),
                    static_cast<int>(strlen(wmName)));

    // Set _NET_NUMBER_OF_DESKTOPS
    long numDesktops = wm_.numDesktops();
    XChangeProperty(dpy, root, netNumberOfDesktops_, XA_CARDINAL, 32,
                    PropModeReplace, reinterpret_cast<unsigned char*>(&numDesktops), 1);

    // Set _NET_CURRENT_DESKTOP
    long curDesktop = wm_.currentDesktop();
    XChangeProperty(dpy, root, netCurrentDesktop_, XA_CARDINAL, 32,
                    PropModeReplace, reinterpret_cast<unsigned char*>(&curDesktop), 1);

    // Set _NET_DESKTOP_NAMES
    Atom netDesktopNames = XInternAtom(dpy, "_NET_DESKTOP_NAMES", False);
    std::string names;
    for (int i = 0; i < wm_.numDesktops(); ++i) {
        names += "Desktop " + std::to_string(i + 1);
        names += '\0';
    }
    XChangeProperty(dpy, root, netDesktopNames, utf8String_, 8,
                    PropModeReplace,
                    reinterpret_cast<const unsigned char*>(names.data()),
                    static_cast<int>(names.size()));

    setSupportedAtoms();
    updateWorkarea();
}

void EWMH::setSupportedAtoms() {
    std::vector<Atom> supported = {
        netSupported_, netClientList_, netClientListStacking_,
        netActiveWindow_, netWmName_, netWmDesktop_,
        netWmState_, netWmStateFullscreen_,
        netWmStateMaximizedVert_, netWmStateMaximizedHorz_,
        netWmStateHidden_, netWmStateAbove_,
        netWmWindowType_, netWmWindowTypeNormal_,
        netWmWindowTypeDialog_, netWmWindowTypeDock_,
        netWmAllowedActions_, netWmActionMove_, netWmActionResize_,
        netWmActionMinimize_, netWmActionMaximizeHorz_,
        netWmActionMaximizeVert_, netWmActionClose_,
        netWmActionFullscreen_, netCloseWindow_,
        netWorkarea_, netSupportingWmCheck_,
        netNumberOfDesktops_, netCurrentDesktop_
    };

    XChangeProperty(wm_.display(), wm_.root(), netSupported_, XA_ATOM, 32,
                    PropModeReplace,
                    reinterpret_cast<unsigned char*>(supported.data()),
                    static_cast<int>(supported.size()));
}

void EWMH::updateClientList() {
    std::vector<Window> clients;
    for (auto& [w, c] : wm_.clients()) {
        clients.push_back(w);
    }

    auto* dpy = wm_.display();
    auto root = wm_.root();

    XChangeProperty(dpy, root, netClientList_, XA_WINDOW, 32,
                    PropModeReplace,
                    reinterpret_cast<unsigned char*>(clients.data()),
                    static_cast<int>(clients.size()));

    // Stacking order (simplified: same as client list)
    XChangeProperty(dpy, root, netClientListStacking_, XA_WINDOW, 32,
                    PropModeReplace,
                    reinterpret_cast<unsigned char*>(clients.data()),
                    static_cast<int>(clients.size()));
}

void EWMH::updateActiveWindow(Window client) {
    XChangeProperty(wm_.display(), wm_.root(), netActiveWindow_, XA_WINDOW, 32,
                    PropModeReplace,
                    reinterpret_cast<unsigned char*>(&client), 1);
}

void EWMH::updateWindowState(ClientWindow* client) {
    if (!client) return;

    std::vector<Atom> states;
    if (client->maximized) {
        states.push_back(netWmStateMaximizedVert_);
        states.push_back(netWmStateMaximizedHorz_);
    }
    if (client->iconified) {
        states.push_back(netWmStateHidden_);
    }

    XChangeProperty(wm_.display(), client->client, netWmState_, XA_ATOM, 32,
                    PropModeReplace,
                    reinterpret_cast<unsigned char*>(states.data()),
                    static_cast<int>(states.size()));
}

void EWMH::updateWindowDesktop(ClientWindow* client, int desktop) {
    if (!client) return;
    long d = desktop;
    XChangeProperty(wm_.display(), client->client, netWmDesktop_, XA_CARDINAL, 32,
                    PropModeReplace,
                    reinterpret_cast<unsigned char*>(&d), 1);
}

bool EWMH::handleClientMessage(const XClientMessageEvent& ev) {
    if (ev.message_type == netWmState_) {
        auto* client = wm_.findClient(ev.window);
        if (!client) return false;

        // data.l[0] = action (0=remove, 1=add, 2=toggle)
        // data.l[1], data.l[2] = state atoms
        int action = ev.data.l[0];

        for (int i = 1; i <= 2; ++i) {
            Atom state = static_cast<Atom>(ev.data.l[i]);
            if (state == None) continue;

            bool set = false;
            if (state == netWmStateMaximizedVert_ || state == netWmStateMaximizedHorz_) {
                if (action == 1 || (action == 2 && !client->maximized)) {
                    set = true;
                }
                if (set && !client->maximized) {
                    wm_.maximizeClient(client);
                } else if (!set && client->maximized) {
                    wm_.restoreClient(client);
                }
            } else if (state == netWmStateHidden_) {
                if (action == 1 || (action == 2 && !client->iconified)) {
                    wm_.iconifyClient(client);
                }
            }
        }

        updateWindowState(client);
        return true;
    }

    if (ev.message_type == netActiveWindow_) {
        auto* client = wm_.findClient(ev.window);
        if (client) {
            if (client->iconified) {
                client->iconified = false;
                XMapWindow(wm_.display(), client->frame);
            }
            wm_.focusClient(client);
        }
        return true;
    }

    if (ev.message_type == netCloseWindow_) {
        auto* client = wm_.findClient(ev.window);
        if (client) {
            wm_.closeClient(client);
        }
        return true;
    }

    if (ev.message_type == netCurrentDesktop_) {
        int desktop = static_cast<int>(ev.data.l[0]);
        wm_.switchDesktop(desktop);
        return true;
    }

    if (ev.message_type == netWmDesktop_) {
        auto* client = wm_.findClient(ev.window);
        if (client) {
            int desktop = static_cast<int>(ev.data.l[0]);
            wm_.moveClientToDesktop(client, desktop);
        }
        return true;
    }

    return false;
}

void EWMH::setAllowedActions(ClientWindow* client) {
    if (!client) return;

    std::vector<Atom> actions = {
        netWmActionMove_, netWmActionResize_,
        netWmActionMinimize_, netWmActionMaximizeHorz_,
        netWmActionMaximizeVert_, netWmActionClose_,
        netWmActionFullscreen_
    };

    XChangeProperty(wm_.display(), client->client, netWmAllowedActions_, XA_ATOM, 32,
                    PropModeReplace,
                    reinterpret_cast<unsigned char*>(actions.data()),
                    static_cast<int>(actions.size()));
}

EWMH::WindowType EWMH::readWindowType(Window w) const {
    Atom actualType;
    int actualFormat;
    unsigned long nitems, bytesAfter;
    unsigned char* data = nullptr;

    if (XGetWindowProperty(wm_.display(), w, netWmWindowType_,
                           0, 1, False, XA_ATOM,
                           &actualType, &actualFormat,
                           &nitems, &bytesAfter, &data) == Success && data) {
        Atom type = *reinterpret_cast<Atom*>(data);
        XFree(data);

        if (type == netWmWindowTypeDialog_)  return WindowType::Dialog;
        if (type == netWmWindowTypeDock_)    return WindowType::Dock;
        if (type == netWmWindowTypeToolbar_) return WindowType::Toolbar;
        if (type == netWmWindowTypeMenu_)    return WindowType::Menu;
        if (type == netWmWindowTypeUtility_) return WindowType::Utility;
        if (type == netWmWindowTypeSplash_)  return WindowType::Splash;
        if (type == netWmWindowTypeDesktop_) return WindowType::Desktop;
    }

    return WindowType::Normal;
}

EWMH::Struts EWMH::readStruts(Window w) const {
    Struts struts;
    Atom actualType;
    int actualFormat;
    unsigned long nitems, bytesAfter;
    unsigned char* data = nullptr;

    // Try _NET_WM_STRUT_PARTIAL first (12 values), fall back to _NET_WM_STRUT (4 values)
    if (XGetWindowProperty(wm_.display(), w, netWmStrutPartial_,
                           0, 12, False, XA_CARDINAL,
                           &actualType, &actualFormat,
                           &nitems, &bytesAfter, &data) == Success && data && nitems >= 4) {
        long* vals = reinterpret_cast<long*>(data);
        struts.left   = static_cast<int>(vals[0]);
        struts.right  = static_cast<int>(vals[1]);
        struts.top    = static_cast<int>(vals[2]);
        struts.bottom = static_cast<int>(vals[3]);
        XFree(data);
        return struts;
    }
    if (data) XFree(data);

    if (XGetWindowProperty(wm_.display(), w, netWmStrut_,
                           0, 4, False, XA_CARDINAL,
                           &actualType, &actualFormat,
                           &nitems, &bytesAfter, &data) == Success && data && nitems >= 4) {
        long* vals = reinterpret_cast<long*>(data);
        struts.left   = static_cast<int>(vals[0]);
        struts.right  = static_cast<int>(vals[1]);
        struts.top    = static_cast<int>(vals[2]);
        struts.bottom = static_cast<int>(vals[3]);
        XFree(data);
    } else if (data) {
        XFree(data);
    }

    return struts;
}

void EWMH::updateWorkarea() {
    int screenW = DisplayWidth(wm_.display(), wm_.screen());
    int screenH = DisplayHeight(wm_.display(), wm_.screen());

    // Aggregate struts from all dock-type windows
    Struts total;
    for (auto& [w, c] : wm_.clients()) {
        if (readWindowType(w) == WindowType::Dock) {
            auto s = readStruts(w);
            total.left   = std::max(total.left, s.left);
            total.right  = std::max(total.right, s.right);
            total.top    = std::max(total.top, s.top);
            total.bottom = std::max(total.bottom, s.bottom);
        }
    }

    // Workarea is per-desktop (4 longs each)
    std::vector<long> workarea;
    for (int i = 0; i < wm_.numDesktops(); ++i) {
        workarea.push_back(total.left);
        workarea.push_back(total.top);
        workarea.push_back(screenW - total.left - total.right);
        workarea.push_back(screenH - total.top - total.bottom);
    }

    XChangeProperty(wm_.display(), wm_.root(), netWorkarea_, XA_CARDINAL, 32,
                    PropModeReplace,
                    reinterpret_cast<unsigned char*>(workarea.data()),
                    static_cast<int>(workarea.size()));
}

} // namespace motif::wm
