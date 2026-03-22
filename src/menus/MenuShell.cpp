#include <motif/menus/MenuShell.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace motif {

MenuShell::MenuShell(Widget* parent) : Shell(parent) {}
MenuShell::~MenuShell() = default;

void MenuShell::createWindow() {
    auto& app = Application::instance();
    auto* dpy = app.display();
    if (!dpy) return;

    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    attrs.background_pixel = app.whitePixel();
    attrs.border_pixel = app.blackPixel();
    attrs.save_under = True;
    attrs.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask |
                       EnterWindowMask | LeaveWindowMask | PointerMotionMask;

    window_ = XCreateWindow(
        dpy, app.rootWindow(),
        x_, y_, width_, height_,
        1,
        CopyFromParent,
        InputOutput,
        CopyFromParent,
        CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWSaveUnder | CWEventMask,
        &attrs
    );

    gc_ = XCreateGC(dpy, window_, 0, nullptr);
    app.registerWidget(window_, this);
}

void MenuShell::popup(int x, int y) {
    x_ = x; y_ = y;
    if (!realized_) realize();

    auto* dpy = Application::instance().display();
    if (dpy && window_) {
        XMoveWindow(dpy, window_, x, y);
        XMapRaised(dpy, window_);
        visible_ = true;

        XGrabPointer(dpy, window_, True,
                     ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                     GrabModeAsync, GrabModeAsync,
                     None, None, CurrentTime);
    }
}

void MenuShell::popdown() {
    auto* dpy = Application::instance().display();
    if (dpy) {
        XUngrabPointer(dpy, CurrentTime);
        if (window_) XUnmapWindow(dpy, window_);
    }
    visible_ = false;
}

} // namespace motif
