#include <motif/dnd/DragIcon.h>
#include <motif/core/Application.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace motif {

DragIcon::DragIcon() = default;

DragIcon::~DragIcon() {
    destroy();
}

void DragIcon::create() {
    if (window_) return;

    auto& app = Application::instance();
    auto* dpy = app.display();
    if (!dpy) return;

    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    attrs.background_pixel = app.whitePixel();
    attrs.border_pixel = app.blackPixel();
    attrs.save_under = True;

    window_ = XCreateWindow(
        dpy, app.rootWindow(),
        0, 0, width_, height_,
        1,
        CopyFromParent,
        InputOutput,
        CopyFromParent,
        CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWSaveUnder,
        &attrs
    );

    gc_ = XCreateGC(dpy, window_, 0, nullptr);
}

void DragIcon::destroy() {
    auto* dpy = Application::instance().display();
    if (!dpy) return;

    if (gc_) { XFreeGC(dpy, gc_); gc_ = nullptr; }
    if (window_) { XDestroyWindow(dpy, window_); window_ = 0; }
    visible_ = false;
}

void DragIcon::setState(State state) {
    if (state_ == state) return;
    state_ = state;
    if (visible_) redraw();
}

void DragIcon::setPixmapData(const std::vector<uint8_t>& /*data*/, int /*w*/, int /*h*/) {
    iconType_ = IconType::Custom;
    // Pixmap rendering reserved for future implementation
}

void DragIcon::show(int rootX, int rootY) {
    if (!window_) create();

    auto* dpy = Application::instance().display();
    if (!dpy || !window_) return;

    XMoveWindow(dpy, window_, rootX + 12, rootY + 12);
    XMapRaised(dpy, window_);
    visible_ = true;
    redraw();
}

void DragIcon::redraw() {
    auto* dpy = Application::instance().display();
    if (!dpy || !window_ || !gc_) return;

    auto& app = Application::instance();
    Colormap cmap = DefaultColormap(dpy, app.screen());

    // Background color based on state
    unsigned long bg = app.whitePixel();
    unsigned long borderColor = app.blackPixel();

    if (state_ == State::Accept) {
        XColor color, exact;
        if (XAllocNamedColor(dpy, cmap, "#E8F5E9", &color, &exact))
            bg = color.pixel;
        if (XAllocNamedColor(dpy, cmap, "#4CAF50", &color, &exact))
            borderColor = color.pixel;
    } else if (state_ == State::Reject) {
        XColor color, exact;
        if (XAllocNamedColor(dpy, cmap, "#FFEBEE", &color, &exact))
            bg = color.pixel;
        if (XAllocNamedColor(dpy, cmap, "#F44336", &color, &exact))
            borderColor = color.pixel;
    }

    XSetWindowBackground(dpy, window_, bg);
    XSetWindowBorder(dpy, window_, borderColor);
    XClearWindow(dpy, window_);

    // Draw text
    XSetForeground(dpy, gc_, app.blackPixel());
    XFontStruct* font = XQueryFont(dpy, XGContextFromGC(gc_));
    int textX = 4;
    int textY = font ? (font->ascent + 4) : 14;

    // Draw icon type indicator
    switch (iconType_) {
        case IconType::Copy: {
            // Draw "+" before text
            XSetLineAttributes(dpy, gc_, 2, LineSolid, CapRound, JoinRound);
            XDrawLine(dpy, window_, gc_, 6, 8, 6, 18);
            XDrawLine(dpy, window_, gc_, 1, 13, 11, 13);
            XSetLineAttributes(dpy, gc_, 1, LineSolid, CapButt, JoinMiter);
            textX = 16;
            break;
        }
        case IconType::Move: {
            // Draw arrow before text
            XPoint arrow[] = {{4, 14}, {10, 6}, {10, 10}, {16, 10},
                              {16, 18}, {10, 18}, {10, 22}};
            XFillPolygon(dpy, window_, gc_, arrow, 7, Nonconvex, CoordModeOrigin);
            textX = 20;
            break;
        }
        case IconType::Link: {
            // Draw chain-link icon
            XDrawArc(dpy, window_, gc_, 2, 6, 8, 12, 0, 360 * 64);
            XDrawArc(dpy, window_, gc_, 8, 8, 8, 12, 0, 360 * 64);
            textX = 20;
            break;
        }
        default:
            break;
    }

    if (!text_.empty()) {
        XDrawString(dpy, window_, gc_, textX, textY,
                    text_.c_str(), static_cast<int>(text_.size()));
    }

    // Draw state indicator in top-right corner
    drawStateIndicator();

    if (font) XFreeFontInfo(nullptr, font, 0);
}

void DragIcon::drawStateIndicator() {
    auto* dpy = Application::instance().display();
    if (!dpy || !window_ || !gc_) return;

    auto& app = Application::instance();
    Colormap cmap = DefaultColormap(dpy, app.screen());
    int ix = width_ - 12;
    int iy = 2;

    if (state_ == State::Accept) {
        // Green checkmark
        XColor color, exact;
        unsigned long green = app.blackPixel();
        if (XAllocNamedColor(dpy, cmap, "#4CAF50", &color, &exact))
            green = color.pixel;
        XSetForeground(dpy, gc_, green);
        XSetLineAttributes(dpy, gc_, 2, LineSolid, CapRound, JoinRound);
        XDrawLine(dpy, window_, gc_, ix, iy + 6, ix + 3, iy + 9);
        XDrawLine(dpy, window_, gc_, ix + 3, iy + 9, ix + 8, iy + 2);
        XSetLineAttributes(dpy, gc_, 1, LineSolid, CapButt, JoinMiter);
    } else if (state_ == State::Reject) {
        // Red X
        XColor color, exact;
        unsigned long red = app.blackPixel();
        if (XAllocNamedColor(dpy, cmap, "#F44336", &color, &exact))
            red = color.pixel;
        XSetForeground(dpy, gc_, red);
        XSetLineAttributes(dpy, gc_, 2, LineSolid, CapRound, JoinRound);
        XDrawLine(dpy, window_, gc_, ix, iy + 1, ix + 8, iy + 9);
        XDrawLine(dpy, window_, gc_, ix + 8, iy + 1, ix, iy + 9);
        XSetLineAttributes(dpy, gc_, 1, LineSolid, CapButt, JoinMiter);
    }
}

void DragIcon::hide() {
    if (!window_ || !visible_) return;

    auto* dpy = Application::instance().display();
    if (dpy) {
        XUnmapWindow(dpy, window_);
    }
    visible_ = false;
}

void DragIcon::moveTo(int rootX, int rootY) {
    if (!window_ || !visible_) return;

    auto* dpy = Application::instance().display();
    if (dpy) {
        XMoveWindow(dpy, window_, rootX + 12, rootY + 12);
    }
}

} // namespace motif
