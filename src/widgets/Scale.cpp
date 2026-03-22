#include <motif/widgets/Scale.h>
#include <motif/core/Application.h>

#include <X11/Xlib.h>
#include <algorithm>
#include <cmath>
#include <string>

namespace motif {

Scale::Scale(Widget* parent) : Primitive(parent) {
    shadowThickness_ = 1;
    resources_.set<int>("orientation", static_cast<int>(Orientation::Horizontal));
    resources_.set<int>("value", 0);
    resources_.set<int>("minimum", 0);
    resources_.set<int>("maximum", 100);
    resources_.set<bool>("showValue", true);
    resources_.set<int>("decimalPoints", 0);
    resources_.set<std::string>("title", "");
}

Scale::~Scale() = default;

std::vector<ResourceSpec> Scale::resourceSpecs() const {
    auto specs = Primitive::resourceSpecs();
    specs.emplace_back("orientation", "Orientation", typeid(int), 0,
                       ResourceValue{static_cast<int>(Orientation::Horizontal)});
    specs.emplace_back("value", "Value", typeid(int), 0, ResourceValue{0});
    specs.emplace_back("minimum", "Minimum", typeid(int), 0, ResourceValue{0});
    specs.emplace_back("maximum", "Maximum", typeid(int), 0, ResourceValue{100});
    specs.emplace_back("showValue", "ShowValue", typeid(bool), 0, ResourceValue{true});
    specs.emplace_back("decimalPoints", "DecimalPoints", typeid(int), 0, ResourceValue{0});
    specs.emplace_back("title", "Title", typeid(std::string), 0,
                       ResourceValue{std::string("")});
    return specs;
}

void Scale::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "orientation") {
        orientation_ = static_cast<Orientation>(resources_.get<int>("orientation", 0));
    } else if (resourceName == "value") {
        value_ = resources_.get<int>("value", 0);
    } else if (resourceName == "minimum") {
        minimum_ = resources_.get<int>("minimum", 0);
    } else if (resourceName == "maximum") {
        maximum_ = resources_.get<int>("maximum", 100);
    } else if (resourceName == "showValue") {
        showValue_ = resources_.get<bool>("showValue", true);
    } else if (resourceName == "decimalPoints") {
        decimalPoints_ = resources_.get<int>("decimalPoints", 0);
    } else if (resourceName == "title") {
        title_ = resources_.get<std::string>("title", "");
    }
    Primitive::onResourceChanged(resourceName);
}

void Scale::setValue(int value) {
    value_ = std::clamp(value, minimum_, maximum_);
    invokeCallbacks(ValueChangedCallback);
    if (realized_) handleExpose();
}

int Scale::troughStart() const {
    return kSliderSize / 2;
}

int Scale::troughLength() const {
    bool vert = (orientation_ == Orientation::Vertical);
    int total = vert ? (height_ - kLabelArea) : (width_ - kLabelArea);
    return total - kSliderSize;
}

int Scale::sliderPixelPos() const {
    int range = maximum_ - minimum_;
    if (range <= 0) return 0;
    return troughLength() * (value_ - minimum_) / range;
}

int Scale::pixelToValue(int pixel) const {
    int tLen = troughLength();
    if (tLen <= 0) return minimum_;
    int range = maximum_ - minimum_;
    int val = minimum_ + range * pixel / tLen;
    return std::clamp(val, minimum_, maximum_);
}

void Scale::expose() {
    if (!window_ || !gc_) return;
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();

    XClearWindow(dpy, window_);

    bool vert = (orientation_ == Orientation::Vertical);
    int tStart = troughStart();
    int tLen = troughLength();
    int sPos = sliderPixelPos();

    if (vert) {
        // Track (vertical center)
        int trackX = width_ / 2 - kTrackThickness / 2;
        XSetForeground(dpy, gc_, app.blackPixel());
        XDrawRectangle(dpy, window_, gc_, trackX, tStart, kTrackThickness, tLen);

        // Slider
        int sliderY = tStart + sPos - kSliderSize / 2;
        XSetForeground(dpy, gc_, app.whitePixel());
        XFillRectangle(dpy, window_, gc_, trackX - 4, sliderY, kTrackThickness + 8, kSliderSize);
        XSetForeground(dpy, gc_, app.blackPixel());
        XDrawRectangle(dpy, window_, gc_, trackX - 4, sliderY, kTrackThickness + 8, kSliderSize);
    } else {
        // Track (horizontal center of trough area)
        int trackY = (height_ - kLabelArea) / 2 - kTrackThickness / 2;
        XSetForeground(dpy, gc_, app.blackPixel());
        XDrawRectangle(dpy, window_, gc_, tStart, trackY, tLen, kTrackThickness);

        // Slider
        int sliderX = tStart + sPos - kSliderSize / 2;
        XSetForeground(dpy, gc_, app.whitePixel());
        XFillRectangle(dpy, window_, gc_, sliderX, trackY - 4, kSliderSize, kTrackThickness + 8);
        XSetForeground(dpy, gc_, app.blackPixel());
        XDrawRectangle(dpy, window_, gc_, sliderX, trackY - 4, kSliderSize, kTrackThickness + 8);
    }

    // Value label
    if (showValue_) {
        std::string valStr;
        if (decimalPoints_ > 0) {
            double dval = value_ / std::pow(10.0, decimalPoints_);
            valStr = std::to_string(dval);
            // Trim trailing zeros after decimal
            size_t dot = valStr.find('.');
            if (dot != std::string::npos) {
                valStr = valStr.substr(0, dot + decimalPoints_ + 1);
            }
        } else {
            valStr = std::to_string(value_);
        }

        XFontStruct* font = XQueryFont(dpy, XGContextFromGC(gc_));
        if (font) {
            int tw = XTextWidth(font, valStr.c_str(), static_cast<int>(valStr.size()));
            int asc = font->ascent;
            XSetForeground(dpy, gc_, foreground_);

            if (vert) {
                XDrawString(dpy, window_, gc_, (width_ - tw) / 2, height_ - 4,
                            valStr.c_str(), static_cast<int>(valStr.size()));
            } else {
                XDrawString(dpy, window_, gc_, (width_ - tw) / 2, height_ - kLabelArea + asc + 2,
                            valStr.c_str(), static_cast<int>(valStr.size()));
            }
            XFreeFontInfo(nullptr, font, 0);
        }
    }

    // Title
    if (!title_.empty()) {
        XFontStruct* font = XQueryFont(dpy, XGContextFromGC(gc_));
        if (font) {
            int tw = XTextWidth(font, title_.c_str(), static_cast<int>(title_.size()));
            int asc = font->ascent;
            XSetForeground(dpy, gc_, foreground_);

            if (vert) {
                XDrawString(dpy, window_, gc_, (width_ - tw) / 2, asc + 2,
                            title_.c_str(), static_cast<int>(title_.size()));
            } else {
                XDrawString(dpy, window_, gc_, 2, asc + 2,
                            title_.c_str(), static_cast<int>(title_.size()));
            }
            XFreeFontInfo(nullptr, font, 0);
        }
    }
}

void Scale::handleButtonPress(unsigned int button, int bx, int by) {
    if (button != 1 || !sensitive_) return;

    bool vert = (orientation_ == Orientation::Vertical);
    int clickPos = vert ? by : bx;
    int tStart = troughStart();
    int sPos = tStart + sliderPixelPos();

    // Check if clicking on slider
    if (clickPos >= sPos - kSliderSize / 2 && clickPos <= sPos + kSliderSize / 2) {
        dragging_ = true;
        dragOffset_ = clickPos - sPos;
    } else {
        // Jump to clicked position
        int relPos = clickPos - tStart;
        setValue(pixelToValue(relPos));
    }
}

void Scale::handleButtonRelease(unsigned int button, int /*x*/, int /*y*/) {
    if (button == 1) {
        dragging_ = false;
    }
}

void Scale::handleMotionNotify(int x, int y, unsigned int /*state*/) {
    if (!dragging_) return;

    bool vert = (orientation_ == Orientation::Vertical);
    int pos = vert ? y : x;
    int relPos = pos - troughStart() - dragOffset_;
    int newVal = pixelToValue(relPos);

    if (newVal != value_) {
        value_ = newVal;
        invokeCallbacks(DragCallback);
        if (realized_) handleExpose();
    }
}

} // namespace motif
