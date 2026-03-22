#include <motif/menus/MenuBar.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>

namespace motif {

MenuBar::MenuBar(Widget* parent) : Manager(parent) {}
MenuBar::~MenuBar() = default;

void MenuBar::layout() {
    // Horizontal layout: each child gets equal width, help widget pushed right
    if (children_.empty()) return;

    int n = static_cast<int>(children_.size());
    int helpIdx = -1;

    if (helpWidget_) {
        for (int i = 0; i < n; ++i) {
            if (children_[i] == helpWidget_) { helpIdx = i; break; }
        }
    }

    int normalCount = helpIdx >= 0 ? n - 1 : n;
    int btnW = normalCount > 0 ? std::min(120, width_ / normalCount) : width_;
    int x = 0;

    for (int i = 0; i < n; ++i) {
        if (i == helpIdx) continue;
        children_[i]->setPosition(x, 0, btnW, height_);
        x += btnW;
    }

    if (helpIdx >= 0 && helpWidget_) {
        helpWidget_->setPosition(width_ - btnW, 0, btnW, height_);
    }
}

void MenuBar::expose() {
    if (!window_ || !gc_) return;
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();

    XClearWindow(dpy, window_);

    // Draw bottom shadow line
    XSetForeground(dpy, gc_, app.blackPixel());
    XDrawLine(dpy, window_, gc_, 0, height_ - 1, width_, height_ - 1);
    XSetForeground(dpy, gc_, app.whitePixel());
    XDrawLine(dpy, window_, gc_, 0, 0, width_, 0);
}

} // namespace motif
