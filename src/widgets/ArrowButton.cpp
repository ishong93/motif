#include <motif/widgets/ArrowButton.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>

namespace motif {

ArrowButton::ArrowButton(Widget* parent) : Primitive(parent) {
    resources_.set<int>("arrowDirection", static_cast<int>(Direction::Up));
}
ArrowButton::~ArrowButton() = default;

std::vector<ResourceSpec> ArrowButton::resourceSpecs() const {
    auto specs = Primitive::resourceSpecs();
    specs.emplace_back("arrowDirection", "ArrowDirection", typeid(int), 0,
                       ResourceValue{static_cast<int>(Direction::Up)});
    return specs;
}

void ArrowButton::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "arrowDirection") {
        direction_ = static_cast<Direction>(resources_.get<int>("arrowDirection", 0));
    }
    Primitive::onResourceChanged(resourceName);
}

void ArrowButton::expose() {
    if (!window_ || !gc_) return;
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();

    XClearWindow(dpy, window_);

    int cx = width_ / 2;
    int cy = height_ / 2;
    int sz = std::min(width_, height_) / 2 - 2;

    XPoint pts[3];
    switch (direction_) {
        case Direction::Up:
            pts[0] = {(short)cx, (short)(cy - sz)};
            pts[1] = {(short)(cx - sz), (short)(cy + sz)};
            pts[2] = {(short)(cx + sz), (short)(cy + sz)};
            break;
        case Direction::Down:
            pts[0] = {(short)cx, (short)(cy + sz)};
            pts[1] = {(short)(cx - sz), (short)(cy - sz)};
            pts[2] = {(short)(cx + sz), (short)(cy - sz)};
            break;
        case Direction::Left:
            pts[0] = {(short)(cx - sz), (short)cy};
            pts[1] = {(short)(cx + sz), (short)(cy - sz)};
            pts[2] = {(short)(cx + sz), (short)(cy + sz)};
            break;
        case Direction::Right:
            pts[0] = {(short)(cx + sz), (short)cy};
            pts[1] = {(short)(cx - sz), (short)(cy - sz)};
            pts[2] = {(short)(cx - sz), (short)(cy + sz)};
            break;
    }

    XSetForeground(dpy, gc_, armed_ ? app.blackPixel() : foreground_);
    XFillPolygon(dpy, window_, gc_, pts, 3, Convex, CoordModeOrigin);
}

void ArrowButton::handleButtonPress(unsigned int button, int x, int y) {
    if (button == 1 && sensitive_) {
        armed_ = true;
        invokeCallbacks(ArmCallback);
        if (realized_) handleExpose();
    }
}

void ArrowButton::handleButtonRelease(unsigned int button, int x, int y) {
    if (button == 1 && armed_) {
        armed_ = false;
        invokeCallbacks(DisarmCallback);
        if (x >= 0 && x < width_ && y >= 0 && y < height_) {
            invokeCallbacks(ActivateCallback);
        }
        if (realized_) handleExpose();
    }
}

} // namespace motif
