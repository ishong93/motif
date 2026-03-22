#include <motif/dialogs/DialogShell.h>
#include <motif/core/Application.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

namespace motif {

DialogShell::DialogShell(Widget* parent) : Shell(parent) {}
DialogShell::~DialogShell() = default;

void DialogShell::createWindow() {
    Shell::createWindow();

    auto* dpy = Application::instance().display();
    if (!dpy || !window_) return;

    // Set transient-for hint so WM treats this as a dialog
    if (parent_ && parent_->window()) {
        XSetTransientForHint(dpy, window_, parent_->window());
    }

    // Set WM_WINDOW_TYPE to dialog
    Atom netWmType = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    Atom netWmDialog = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    XChangeProperty(dpy, window_, netWmType, XA_ATOM, 32,
                    PropModeReplace, reinterpret_cast<unsigned char*>(&netWmDialog), 1);
}

void DialogShell::popup() {
    if (!realized_) realize();
    show();

    if (modal_) {
        // Simple modal loop: process events until hidden
        auto& app = Application::instance();
        while (visible_ && app.isOpen()) {
            app.processEvent();
        }
    }
}

void DialogShell::popdown() {
    hide();
}

} // namespace motif
