#include <motif/core/Primitive.h>
#include <motif/core/Application.h>

#include <X11/Xlib.h>

namespace motif {

Primitive::Primitive(Widget* parent) : Widget(parent) {
    auto& app = Application::instance();
    foreground_ = app.blackPixel();
    background_ = app.whitePixel();

    resources_.set<unsigned long>("foreground", foreground_);
    resources_.set<unsigned long>("background", background_);
    resources_.set<int>("highlightThickness", highlightThickness_);
    resources_.set<int>("shadowThickness", shadowThickness_);
}

Primitive::~Primitive() = default;

std::vector<ResourceSpec> Primitive::resourceSpecs() const {
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

void Primitive::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "foreground") {
        foreground_ = resources_.get<unsigned long>("foreground", 0UL);
    } else if (resourceName == "background") {
        setBackground(resources_.get<unsigned long>("background", ~0UL));
        return; // setBackground already triggers redraw
    } else if (resourceName == "highlightThickness") {
        highlightThickness_ = resources_.get<int>("highlightThickness", 2);
    } else if (resourceName == "shadowThickness") {
        shadowThickness_ = resources_.get<int>("shadowThickness", 2);
    }
    Widget::onResourceChanged(resourceName);
}

bool Primitive::acceptsFocus() const {
    return sensitive_ && visible_;
}

void Primitive::handleFocusIn() {
    focused_ = true;
    drawHighlight();
}

void Primitive::handleFocusOut() {
    focused_ = false;
    eraseHighlight();
}

void Primitive::setBackground(unsigned long pixel) {
    background_ = pixel;
    if (window_) {
        auto* dpy = Application::instance().display();
        XSetWindowBackground(dpy, window_, background_);
        if (realized_) {
            XClearWindow(dpy, window_);
            handleExpose();
        }
    }
}

void Primitive::drawHighlight() {
    if (!window_ || !gc_ || highlightThickness_ <= 0) return;
    auto* dpy = Application::instance().display();

    XSetForeground(dpy, gc_, foreground_);
    for (int i = 0; i < highlightThickness_; ++i) {
        XDrawRectangle(dpy, window_, gc_, i, i,
                       width_ - 1 - 2 * i, height_ - 1 - 2 * i);
    }
}

void Primitive::eraseHighlight() {
    if (!window_ || !gc_ || highlightThickness_ <= 0) return;
    auto* dpy = Application::instance().display();

    XSetForeground(dpy, gc_, background_);
    for (int i = 0; i < highlightThickness_; ++i) {
        XDrawRectangle(dpy, window_, gc_, i, i,
                       width_ - 1 - 2 * i, height_ - 1 - 2 * i);
    }
}

void Primitive::drawShadow(bool raised) {
    if (!window_ || !gc_ || shadowThickness_ <= 0) return;
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();

    int offset = highlightThickness_;
    int w = width_ - 1 - 2 * offset;
    int h = height_ - 1 - 2 * offset;

    // Top-left = light, bottom-right = dark (raised)
    // Swap for sunken
    unsigned long topColor = raised ? app.whitePixel() : app.blackPixel();
    unsigned long botColor = raised ? app.blackPixel() : app.whitePixel();

    for (int i = 0; i < shadowThickness_; ++i) {
        int ox = offset + i;
        int oy = offset + i;
        int ow = w - 2 * i;
        int oh = h - 2 * i;

        // Top edge
        XSetForeground(dpy, gc_, topColor);
        XDrawLine(dpy, window_, gc_, ox, oy, ox + ow, oy);
        // Left edge
        XDrawLine(dpy, window_, gc_, ox, oy, ox, oy + oh);

        // Bottom edge
        XSetForeground(dpy, gc_, botColor);
        XDrawLine(dpy, window_, gc_, ox, oy + oh, ox + ow, oy + oh);
        // Right edge
        XDrawLine(dpy, window_, gc_, ox + ow, oy, ox + ow, oy + oh);
    }
}

} // namespace motif
