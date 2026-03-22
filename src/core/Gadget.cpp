#include <motif/core/Gadget.h>
#include <motif/core/Application.h>

#include <X11/Xlib.h>

namespace motif {

Gadget::Gadget(Widget* parent) : Widget(parent) {
    auto& app = Application::instance();
    foreground_ = app.blackPixel();
    background_ = app.whitePixel();

    resources_.set<unsigned long>("foreground", foreground_);
    resources_.set<unsigned long>("background", background_);
    resources_.set<int>("highlightThickness", highlightThickness_);
    resources_.set<int>("shadowThickness", shadowThickness_);
}

Gadget::~Gadget() = default;

void Gadget::realize() {
    // Gadgets don't create their own X window; they share the parent's.
    realized_ = true;
    for (auto* child : children_) {
        child->realize();
    }
}

void Gadget::show() {
    visible_ = true;
    // Request parent to redraw the area occupied by this gadget
    if (parent_ && parent_->isRealized()) {
        parent_->handleExpose();
    }
}

void Gadget::hide() {
    visible_ = false;
    if (parent_ && parent_->isRealized()) {
        parent_->handleExpose();
    }
}

void Gadget::move(int x, int y) {
    x_ = x;
    y_ = y;
    // No XMoveWindow — just request parent repaint
    if (parent_ && parent_->isRealized()) {
        parent_->handleExpose();
    }
}

void Gadget::resize(int width, int height) {
    width_ = width;
    height_ = height;
    if (parent_ && parent_->isRealized()) {
        parent_->handleExpose();
    }
}

bool Gadget::containsPoint(int px, int py) const {
    return visible_ && sensitive_
        && px >= x_ && px < x_ + width_
        && py >= y_ && py < y_ + height_;
}

XWindow Gadget::parentWindow() const {
    return parent_ ? parent_->window() : 0;
}

XGC Gadget::parentGC() const {
    return parent_ ? parent_->gc() : nullptr;
}

std::vector<ResourceSpec> Gadget::resourceSpecs() const {
    auto specs = Widget::resourceSpecs();
    specs.emplace_back("foreground", "Foreground", typeid(unsigned long), 0,
                       ResourceValue{0UL});
    specs.emplace_back("background", "Background", typeid(unsigned long), 0,
                       ResourceValue{~0UL});
    specs.emplace_back("highlightThickness", "HighlightThickness", typeid(int), 0,
                       ResourceValue{2});
    specs.emplace_back("shadowThickness", "ShadowThickness", typeid(int), 0,
                       ResourceValue{2});
    return specs;
}

void Gadget::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "foreground") {
        foreground_ = resources_.get<unsigned long>("foreground", 0UL);
    } else if (resourceName == "background") {
        background_ = resources_.get<unsigned long>("background", ~0UL);
    } else if (resourceName == "highlightThickness") {
        highlightThickness_ = resources_.get<int>("highlightThickness", 2);
    } else if (resourceName == "shadowThickness") {
        shadowThickness_ = resources_.get<int>("shadowThickness", 2);
    }
    Widget::onResourceChanged(resourceName);
}

void Gadget::expose() {
    auto win = parentWindow();
    auto gc = parentGC();
    if (!win || !gc) return;

    auto* dpy = Application::instance().display();

    // Fill background rectangle in parent's window
    XSetForeground(dpy, gc, background_);
    XFillRectangle(dpy, win, gc, x_, y_, width_, height_);

    // Draw shadow border
    drawShadow(true);

    // Draw highlight if focused
    if (focused_) {
        drawHighlight();
    }
}

void Gadget::drawHighlight() {
    auto win = parentWindow();
    auto gc = parentGC();
    if (!win || !gc || highlightThickness_ <= 0) return;

    auto* dpy = Application::instance().display();
    XSetForeground(dpy, gc, foreground_);
    for (int i = 0; i < highlightThickness_; ++i) {
        XDrawRectangle(dpy, win, gc,
                       x_ + i, y_ + i,
                       width_ - 1 - 2 * i, height_ - 1 - 2 * i);
    }
}

void Gadget::eraseHighlight() {
    auto win = parentWindow();
    auto gc = parentGC();
    if (!win || !gc || highlightThickness_ <= 0) return;

    auto* dpy = Application::instance().display();
    XSetForeground(dpy, gc, background_);
    for (int i = 0; i < highlightThickness_; ++i) {
        XDrawRectangle(dpy, win, gc,
                       x_ + i, y_ + i,
                       width_ - 1 - 2 * i, height_ - 1 - 2 * i);
    }
}

void Gadget::drawShadow(bool raised) {
    auto win = parentWindow();
    auto gc = parentGC();
    if (!win || !gc || shadowThickness_ <= 0) return;

    auto* dpy = Application::instance().display();
    auto& app = Application::instance();

    int offset = highlightThickness_;
    int sx = x_ + offset;
    int sy = y_ + offset;
    int sw = width_ - 1 - 2 * offset;
    int sh = height_ - 1 - 2 * offset;

    unsigned long topColor = raised ? app.whitePixel() : app.blackPixel();
    unsigned long botColor = raised ? app.blackPixel() : app.whitePixel();

    for (int i = 0; i < shadowThickness_; ++i) {
        int ox = sx + i;
        int oy = sy + i;
        int ow = sw - 2 * i;
        int oh = sh - 2 * i;

        XSetForeground(dpy, gc, topColor);
        XDrawLine(dpy, win, gc, ox, oy, ox + ow, oy);
        XDrawLine(dpy, win, gc, ox, oy, ox, oy + oh);

        XSetForeground(dpy, gc, botColor);
        XDrawLine(dpy, win, gc, ox, oy + oh, ox + ow, oy + oh);
        XDrawLine(dpy, win, gc, ox + ow, oy, ox + ow, oy + oh);
    }
}

} // namespace motif
