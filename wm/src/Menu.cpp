#include <motif/wm/Menu.h>
#include <motif/wm/WindowManager.h>

#include <X11/Xutil.h>
#include <algorithm>
#include <cstring>

namespace motif::wm {

Menu::Menu(WindowManager& wm) : wm_(wm) {}

Menu::~Menu() {
    if (font_) {
        XFreeFont(wm_.display(), font_);
        font_ = nullptr;
    }
    if (menuWindow_ != None) {
        XDestroyWindow(wm_.display(), menuWindow_);
    }
    if (gc_) {
        XFreeGC(wm_.display(), gc_);
    }
}

void Menu::buildRootMenu() {
    // Root menu is built dynamically when shown, but we set up defaults
}

void Menu::buildWindowMenu() {
    // Window menu is built dynamically when shown
}

void Menu::createMenuWindow() {
    if (menuWindow_ != None) return;

    auto* dpy = wm_.display();
    XSetWindowAttributes attrs;
    attrs.background_pixel = WhitePixel(dpy, wm_.screen());
    attrs.border_pixel = BlackPixel(dpy, wm_.screen());
    attrs.override_redirect = True;
    attrs.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask |
                       PointerMotionMask | LeaveWindowMask;
    attrs.save_under = True;

    menuWindow_ = XCreateWindow(
        dpy, wm_.root(),
        0, 0, 1, 1, 1,
        CopyFromParent, InputOutput, CopyFromParent,
        CWBackPixel | CWBorderPixel | CWOverrideRedirect |
        CWEventMask | CWSaveUnder, &attrs
    );

    gc_ = XCreateGC(dpy, menuWindow_, 0, nullptr);
    font_ = XLoadQueryFont(dpy, wm_.config().fontName.c_str());
    if (!font_) {
        font_ = XLoadQueryFont(dpy, "fixed");
    }
    if (font_) {
        XSetFont(dpy, gc_, font_->fid);
    }
}

void Menu::showRootMenu(int x, int y) {
    items_.clear();
    items_.push_back({"Restart", [this]() { /* restart WM */ }});
    items_.push_back({"", nullptr, true}); // separator
    items_.push_back({"Quit", [this]() { wm_.quit(); }});

    createMenuWindow();

    // Calculate menu size
    int maxW = 0;
    for (auto& item : items_) {
        if (!item.separator && font_) {
            int w = XTextWidth(font_, item.label.c_str(),
                               static_cast<int>(item.label.size()));
            maxW = std::max(maxW, w);
        }
    }
    int menuW = maxW + 2 * kMenuPadding + 20;
    int menuH = static_cast<int>(items_.size()) * kItemHeight + 2 * kMenuPadding;

    // Clamp to screen
    int screenW = DisplayWidth(wm_.display(), wm_.screen());
    int screenH = DisplayHeight(wm_.display(), wm_.screen());
    if (x + menuW > screenW) x = screenW - menuW;
    if (y + menuH > screenH) y = screenH - menuH;

    XMoveResizeWindow(wm_.display(), menuWindow_, x, y, menuW, menuH);
    XMapRaised(wm_.display(), menuWindow_);
    visible_ = true;
    highlightedItem_ = -1;
    targetClient_ = None;

    XGrabPointer(wm_.display(), menuWindow_, True,
                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
}

void Menu::showWindowMenu(Window client, int x, int y) {
    items_.clear();
    targetClient_ = client;

    auto* c = wm_.findClient(client);

    items_.push_back({"Move", [this]() {
        // Move is handled by title bar drag; this is a placeholder
        hide();
    }});
    items_.push_back({"Resize", [this]() {
        hide();
    }});
    items_.push_back({"", nullptr, true}); // separator

    if (c && c->maximized) {
        items_.push_back({"Restore", [this]() {
            auto* cl = wm_.findClient(targetClient_);
            if (cl) wm_.restoreClient(cl);
            hide();
        }});
    } else {
        items_.push_back({"Maximize", [this]() {
            auto* cl = wm_.findClient(targetClient_);
            if (cl) wm_.maximizeClient(cl);
            hide();
        }});
    }

    items_.push_back({"Minimize", [this]() {
        auto* cl = wm_.findClient(targetClient_);
        if (cl) wm_.iconifyClient(cl);
        hide();
    }});

    items_.push_back({"", nullptr, true}); // separator

    items_.push_back({"Close", [this]() {
        auto* cl = wm_.findClient(targetClient_);
        if (cl) wm_.closeClient(cl);
        hide();
    }});

    createMenuWindow();

    int maxW = 0;
    for (auto& item : items_) {
        if (!item.separator && font_) {
            int w = XTextWidth(font_, item.label.c_str(),
                               static_cast<int>(item.label.size()));
            maxW = std::max(maxW, w);
        }
    }
    int menuW = maxW + 2 * kMenuPadding + 20;
    int menuH = static_cast<int>(items_.size()) * kItemHeight + 2 * kMenuPadding;

    int screenW = DisplayWidth(wm_.display(), wm_.screen());
    int screenH = DisplayHeight(wm_.display(), wm_.screen());
    if (x + menuW > screenW) x = screenW - menuW;
    if (y + menuH > screenH) y = screenH - menuH;

    XMoveResizeWindow(wm_.display(), menuWindow_, x, y, menuW, menuH);
    XMapRaised(wm_.display(), menuWindow_);
    visible_ = true;
    highlightedItem_ = -1;

    XGrabPointer(wm_.display(), menuWindow_, True,
                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
}

void Menu::hide() {
    if (!visible_) return;
    XUngrabPointer(wm_.display(), CurrentTime);
    XUnmapWindow(wm_.display(), menuWindow_);
    visible_ = false;
    highlightedItem_ = -1;
}

void Menu::handleExpose() {
    drawMenu();
}

void Menu::handleButtonPress(int x, int y) {
    int idx = itemAtY(y);
    if (idx >= 0 && idx < static_cast<int>(items_.size()) && !items_[idx].separator) {
        if (items_[idx].action) {
            items_[idx].action();
        }
    }
    hide();
}

void Menu::handleMotionNotify(int x, int y) {
    int idx = itemAtY(y);
    if (idx != highlightedItem_) {
        highlightedItem_ = idx;
        drawMenu();
    }
}

void Menu::drawMenu() {
    if (!visible_ || menuWindow_ == None || !gc_) return;

    auto* dpy = wm_.display();
    auto black = BlackPixel(dpy, wm_.screen());
    auto white = WhitePixel(dpy, wm_.screen());

    // Clear
    XSetForeground(dpy, gc_, white);
    XWindowAttributes wa;
    XGetWindowAttributes(dpy, menuWindow_, &wa);
    XFillRectangle(dpy, menuWindow_, gc_, 0, 0, wa.width, wa.height);

    for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
        int itemY = kMenuPadding + i * kItemHeight;

        if (items_[i].separator) {
            // Draw separator line
            XSetForeground(dpy, gc_, black);
            int lineY = itemY + kItemHeight / 2;
            XDrawLine(dpy, menuWindow_, gc_, kMenuPadding, lineY,
                      wa.width - kMenuPadding, lineY);
            continue;
        }

        // Highlight
        if (i == highlightedItem_) {
            XSetForeground(dpy, gc_, black);
            XFillRectangle(dpy, menuWindow_, gc_, 1, itemY,
                           wa.width - 2, kItemHeight);
            XSetForeground(dpy, gc_, white);
        } else {
            XSetForeground(dpy, gc_, black);
        }

        // Draw label
        if (font_) {
            int textY = itemY + (kItemHeight + font_->ascent - font_->descent) / 2;
            XDrawString(dpy, menuWindow_, gc_, kMenuPadding + 4, textY,
                        items_[i].label.c_str(),
                        static_cast<int>(items_[i].label.size()));
        }
    }

    // 3D border
    XSetForeground(dpy, gc_, white);
    XDrawLine(dpy, menuWindow_, gc_, 0, 0, wa.width - 1, 0);
    XDrawLine(dpy, menuWindow_, gc_, 0, 0, 0, wa.height - 1);
    XSetForeground(dpy, gc_, black);
    XDrawLine(dpy, menuWindow_, gc_, 0, wa.height - 1, wa.width - 1, wa.height - 1);
    XDrawLine(dpy, menuWindow_, gc_, wa.width - 1, 0, wa.width - 1, wa.height - 1);
}

int Menu::itemAtY(int y) const {
    int idx = (y - kMenuPadding) / kItemHeight;
    if (idx < 0 || idx >= static_cast<int>(items_.size())) return -1;
    return idx;
}

} // namespace motif::wm
