#include <motif/widgets/ScrollBar.h>
#include <motif/core/Application.h>

#include <X11/Xlib.h>
#include <algorithm>

namespace motif {

ScrollBar::ScrollBar(Widget* parent) : Primitive(parent) {
    shadowThickness_ = 1;
    resources_.set<int>("orientation", static_cast<int>(Orientation::Vertical));
    resources_.set<int>("value", 0);
    resources_.set<int>("minimum", 0);
    resources_.set<int>("maximum", 100);
    resources_.set<int>("sliderSize", 10);
    resources_.set<int>("increment", 1);
    resources_.set<int>("pageIncrement", 10);
}

ScrollBar::~ScrollBar() = default;

std::vector<ResourceSpec> ScrollBar::resourceSpecs() const {
    auto specs = Primitive::resourceSpecs();
    specs.emplace_back("orientation", "Orientation", typeid(int), 0,
                       ResourceValue{static_cast<int>(Orientation::Vertical)});
    specs.emplace_back("value", "Value", typeid(int), 0, ResourceValue{0});
    specs.emplace_back("minimum", "Minimum", typeid(int), 0, ResourceValue{0});
    specs.emplace_back("maximum", "Maximum", typeid(int), 0, ResourceValue{100});
    specs.emplace_back("sliderSize", "SliderSize", typeid(int), 0, ResourceValue{10});
    specs.emplace_back("increment", "Increment", typeid(int), 0, ResourceValue{1});
    specs.emplace_back("pageIncrement", "PageIncrement", typeid(int), 0, ResourceValue{10});
    return specs;
}

void ScrollBar::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "orientation") {
        orientation_ = static_cast<Orientation>(resources_.get<int>("orientation", 1));
    } else if (resourceName == "value") {
        value_ = resources_.get<int>("value", 0);
    } else if (resourceName == "minimum") {
        minimum_ = resources_.get<int>("minimum", 0);
    } else if (resourceName == "maximum") {
        maximum_ = resources_.get<int>("maximum", 100);
    } else if (resourceName == "sliderSize") {
        sliderSize_ = resources_.get<int>("sliderSize", 10);
    } else if (resourceName == "increment") {
        increment_ = resources_.get<int>("increment", 1);
    } else if (resourceName == "pageIncrement") {
        pageIncrement_ = resources_.get<int>("pageIncrement", 10);
    }
    Primitive::onResourceChanged(resourceName);
}

void ScrollBar::setValue(int value) {
    int maxVal = maximum_ - sliderSize_;
    value_ = std::clamp(value, minimum_, maxVal);
    invokeCallbacks(ValueChangedCallback);
    if (realized_) handleExpose();
}

// ── Geometry Helpers ────────────────────────────────────────

int ScrollBar::arrowSize() const {
    return (orientation_ == Orientation::Vertical) ? width_ : height_;
}

int ScrollBar::troughLength() const {
    int total = (orientation_ == Orientation::Vertical) ? height_ : width_;
    return total - 2 * arrowSize();
}

int ScrollBar::sliderPixelSize() const {
    int range = maximum_ - minimum_;
    if (range <= 0) return troughLength();
    int sz = troughLength() * sliderSize_ / range;
    return std::max(sz, 8);
}

int ScrollBar::sliderPixelPos() const {
    int range = maximum_ - minimum_ - sliderSize_;
    if (range <= 0) return 0;
    int availPixels = troughLength() - sliderPixelSize();
    return availPixels * (value_ - minimum_) / range;
}

int ScrollBar::pixelToValue(int pixel) const {
    int availPixels = troughLength() - sliderPixelSize();
    if (availPixels <= 0) return minimum_;
    int range = maximum_ - minimum_ - sliderSize_;
    int val = minimum_ + range * pixel / availPixels;
    return std::clamp(val, minimum_, maximum_ - sliderSize_);
}

ScrollBar::HitZone ScrollBar::hitTest(int hx, int hy) const {
    int as = arrowSize();
    bool vert = (orientation_ == Orientation::Vertical);
    int pos = vert ? hy : hx;
    int totalLen = vert ? height_ : width_;

    if (pos < as) return HitZone::Arrow1;
    if (pos >= totalLen - as) return HitZone::Arrow2;

    int sliderStart = as + sliderPixelPos();
    int sliderEnd = sliderStart + sliderPixelSize();

    if (pos < sliderStart) return HitZone::SliderBefore;
    if (pos < sliderEnd) return HitZone::Slider;
    return HitZone::SliderAfter;
}

// ── Drawing ─────────────────────────────────────────────────

void ScrollBar::expose() {
    if (!window_ || !gc_) return;
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();

    XClearWindow(dpy, window_);

    bool vert = (orientation_ == Orientation::Vertical);
    int as = arrowSize();

    // Draw trough background
    XSetForeground(dpy, gc_, app.blackPixel());

    // Arrow 1 (up/left)
    if (vert) {
        XSetForeground(dpy, gc_, app.whitePixel());
        XFillRectangle(dpy, window_, gc_, 0, 0, width_, as);
        XSetForeground(dpy, gc_, app.blackPixel());
        XDrawRectangle(dpy, window_, gc_, 0, 0, width_ - 1, as - 1);
        // Triangle pointing up
        XPoint pts[3] = {{(short)(width_/2), (short)(2)},
                         {(short)(2), (short)(as-3)},
                         {(short)(width_-3), (short)(as-3)}};
        XFillPolygon(dpy, window_, gc_, pts, 3, Convex, CoordModeOrigin);
    } else {
        XSetForeground(dpy, gc_, app.whitePixel());
        XFillRectangle(dpy, window_, gc_, 0, 0, as, height_);
        XSetForeground(dpy, gc_, app.blackPixel());
        XDrawRectangle(dpy, window_, gc_, 0, 0, as - 1, height_ - 1);
        XPoint pts[3] = {{(short)(2), (short)(height_/2)},
                         {(short)(as-3), (short)(2)},
                         {(short)(as-3), (short)(height_-3)}};
        XFillPolygon(dpy, window_, gc_, pts, 3, Convex, CoordModeOrigin);
    }

    // Arrow 2 (down/right)
    if (vert) {
        int ay = height_ - as;
        XSetForeground(dpy, gc_, app.whitePixel());
        XFillRectangle(dpy, window_, gc_, 0, ay, width_, as);
        XSetForeground(dpy, gc_, app.blackPixel());
        XDrawRectangle(dpy, window_, gc_, 0, ay, width_ - 1, as - 1);
        XPoint pts[3] = {{(short)(width_/2), (short)(height_-3)},
                         {(short)(2), (short)(ay+2)},
                         {(short)(width_-3), (short)(ay+2)}};
        XFillPolygon(dpy, window_, gc_, pts, 3, Convex, CoordModeOrigin);
    } else {
        int ax = width_ - as;
        XSetForeground(dpy, gc_, app.whitePixel());
        XFillRectangle(dpy, window_, gc_, ax, 0, as, height_);
        XSetForeground(dpy, gc_, app.blackPixel());
        XDrawRectangle(dpy, window_, gc_, ax, 0, as - 1, height_ - 1);
        XPoint pts[3] = {{(short)(width_-3), (short)(height_/2)},
                         {(short)(ax+2), (short)(2)},
                         {(short)(ax+2), (short)(height_-3)}};
        XFillPolygon(dpy, window_, gc_, pts, 3, Convex, CoordModeOrigin);
    }

    // Slider thumb
    int sPos = as + sliderPixelPos();
    int sSize = sliderPixelSize();

    XSetForeground(dpy, gc_, app.whitePixel());
    if (vert) {
        XFillRectangle(dpy, window_, gc_, 1, sPos, width_ - 2, sSize);
        XSetForeground(dpy, gc_, app.blackPixel());
        XDrawRectangle(dpy, window_, gc_, 1, sPos, width_ - 3, sSize - 1);
    } else {
        XFillRectangle(dpy, window_, gc_, sPos, 1, sSize, height_ - 2);
        XSetForeground(dpy, gc_, app.blackPixel());
        XDrawRectangle(dpy, window_, gc_, sPos, 1, sSize - 1, height_ - 3);
    }
}

// ── Events ──────────────────────────────────────────────────

void ScrollBar::handleButtonPress(unsigned int button, int bx, int by) {
    if (button != 1 || !sensitive_) return;

    HitZone zone = hitTest(bx, by);
    bool vert = (orientation_ == Orientation::Vertical);

    switch (zone) {
        case HitZone::Arrow1:
            setValue(value_ - increment_);
            invokeCallbacks(DecrementCallback);
            break;
        case HitZone::Arrow2:
            setValue(value_ + increment_);
            invokeCallbacks(IncrementCallback);
            break;
        case HitZone::SliderBefore:
            setValue(value_ - pageIncrement_);
            invokeCallbacks(PageDecrementCallback);
            break;
        case HitZone::SliderAfter:
            setValue(value_ + pageIncrement_);
            invokeCallbacks(PageIncrementCallback);
            break;
        case HitZone::Slider:
            dragging_ = true;
            dragOffset_ = (vert ? by : bx) - arrowSize() - sliderPixelPos();
            break;
        default:
            break;
    }
}

void ScrollBar::handleButtonRelease(unsigned int button, int /*x*/, int /*y*/) {
    if (button == 1) {
        dragging_ = false;
    }
}

void ScrollBar::handleMotionNotify(int x, int y, unsigned int /*state*/) {
    if (!dragging_) return;

    bool vert = (orientation_ == Orientation::Vertical);
    int pos = vert ? y : x;
    int relPos = pos - arrowSize() - dragOffset_;
    int newVal = pixelToValue(relPos);

    if (newVal != value_) {
        setValue(newVal);
        invokeCallbacks(DragCallback);
    }
}

} // namespace motif
