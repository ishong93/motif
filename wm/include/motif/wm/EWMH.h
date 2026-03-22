#pragma once

#include <X11/Xlib.h>
#include <string>
#include <vector>

namespace motif::wm {

class WindowManager;
struct ClientWindow;

/// Extended Window Manager Hints (EWMH / _NET_WM) protocol support.
/// Implements the freedesktop.org window manager specification for
/// interoperability with modern desktop environments and taskbars.
class EWMH {
public:
    explicit EWMH(WindowManager& wm);

    /// Initialize atoms and set root window properties
    void initialize();

    /// Update _NET_CLIENT_LIST and _NET_CLIENT_LIST_STACKING
    void updateClientList();

    /// Update _NET_ACTIVE_WINDOW
    void updateActiveWindow(Window client);

    /// Update _NET_WM_STATE for a client
    void updateWindowState(ClientWindow* client);

    /// Update _NET_WM_DESKTOP for a client
    void updateWindowDesktop(ClientWindow* client, int desktop);

    /// Handle _NET_WM_STATE client messages (fullscreen, maximized, etc.)
    bool handleClientMessage(const XClientMessageEvent& ev);

    /// Set _NET_WM_ALLOWED_ACTIONS for a client
    void setAllowedActions(ClientWindow* client);

    /// Read _NET_WM_WINDOW_TYPE from client
    enum class WindowType {
        Normal, Dialog, Dock, Toolbar, Menu, Utility, Splash, Desktop
    };
    WindowType readWindowType(Window w) const;

    /// Read _NET_WM_STRUT / _NET_WM_STRUT_PARTIAL (panel reserved space)
    struct Struts { int left = 0, right = 0, top = 0, bottom = 0; };
    Struts readStruts(Window w) const;

    /// Compute workarea from struts
    void updateWorkarea();

    // Atom accessors (for EventHandler)
    Atom netWmState() const { return netWmState_; }
    Atom netActiveWindow() const { return netActiveWindow_; }
    Atom netCloseWindow() const { return netCloseWindow_; }
    Atom netCurrentDesktop() const { return netCurrentDesktop_; }
    Atom netWmDesktop() const { return netWmDesktop_; }

private:
    void setRootProperties();
    void setSupportedAtoms();

    WindowManager& wm_;

    // _NET_SUPPORTED atoms
    Atom netSupported_ = None;
    Atom netClientList_ = None;
    Atom netClientListStacking_ = None;
    Atom netActiveWindow_ = None;
    Atom netWmName_ = None;
    Atom netWmDesktop_ = None;
    Atom netWmState_ = None;
    Atom netWmStateFullscreen_ = None;
    Atom netWmStateMaximizedVert_ = None;
    Atom netWmStateMaximizedHorz_ = None;
    Atom netWmStateHidden_ = None;
    Atom netWmStateAbove_ = None;
    Atom netWmWindowType_ = None;
    Atom netWmWindowTypeNormal_ = None;
    Atom netWmWindowTypeDialog_ = None;
    Atom netWmWindowTypeDock_ = None;
    Atom netWmWindowTypeToolbar_ = None;
    Atom netWmWindowTypeMenu_ = None;
    Atom netWmWindowTypeUtility_ = None;
    Atom netWmWindowTypeSplash_ = None;
    Atom netWmWindowTypeDesktop_ = None;
    Atom netWmAllowedActions_ = None;
    Atom netWmActionMove_ = None;
    Atom netWmActionResize_ = None;
    Atom netWmActionMinimize_ = None;
    Atom netWmActionMaximizeHorz_ = None;
    Atom netWmActionMaximizeVert_ = None;
    Atom netWmActionClose_ = None;
    Atom netWmActionFullscreen_ = None;
    Atom netCloseWindow_ = None;
    Atom netWmStrut_ = None;
    Atom netWmStrutPartial_ = None;
    Atom netWorkarea_ = None;
    Atom netSupportingWmCheck_ = None;
    Atom netNumberOfDesktops_ = None;
    Atom netCurrentDesktop_ = None;
    Atom utf8String_ = None;

    Window wmCheckWin_ = None;
};

} // namespace motif::wm
