#include <motif/widgets/ToggleButton.h>
#include <motif/core/Application.h>

#include <X11/Xlib.h>

namespace motif {

ToggleButton::ToggleButton(Widget* parent) : Label(parent) {
    alignment_ = Alignment::Beginning;
    resources_.set<bool>("set", false);
    resources_.set<int>("indicatorType", static_cast<int>(IndicatorType::CheckBox));
    resources_.set<int>("toggleMode", static_cast<int>(ToggleMode::Boolean));
}

ToggleButton::~ToggleButton() = default;

void ToggleButton::setSelected(bool selected) {
    selected_ = selected;
    resources_.set<bool>("set", selected);
    invokeCallbacks(ValueChangedCallback);
    if (realized_) handleExpose();
}

void ToggleButton::setIndicatorType(IndicatorType type) {
    indicatorType_ = type;
    resources_.set<int>("indicatorType", static_cast<int>(type));
    if (realized_) handleExpose();
}

std::vector<ResourceSpec> ToggleButton::resourceSpecs() const {
    auto specs = Label::resourceSpecs();
    specs.emplace_back("set", "Set", typeid(bool), 0, ResourceValue{false});
    specs.emplace_back("indicatorType", "IndicatorType", typeid(int), 0,
                       ResourceValue{static_cast<int>(IndicatorType::CheckBox)});
    specs.emplace_back("toggleMode", "ToggleMode", typeid(int), 0,
                       ResourceValue{static_cast<int>(ToggleMode::Boolean)});
    return specs;
}

void ToggleButton::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "set") {
        selected_ = resources_.get<bool>("set", false);
    } else if (resourceName == "indicatorType") {
        indicatorType_ = static_cast<IndicatorType>(resources_.get<int>("indicatorType", 0));
    } else if (resourceName == "toggleMode") {
        toggleMode_ = static_cast<ToggleMode>(resources_.get<int>("toggleMode", 0));
    }
    Label::onResourceChanged(resourceName);
}

void ToggleButton::expose() {
    if (!window_ || !gc_) return;
    auto* dpy = Application::instance().display();

    XClearWindow(dpy, window_);

    int border = highlightThickness_ + shadowThickness_;
    int indicatorSize = height_ - 2 * border - 2 * marginHeight_;
    if (indicatorSize < 8) indicatorSize = 8;
    int indX = border + marginWidth_;
    int indY = (height_ - indicatorSize) / 2;

    // Draw indicator
    if (indicatorType_ == IndicatorType::CheckBox) {
        // Square checkbox
        XSetForeground(dpy, gc_, foreground_);
        XDrawRectangle(dpy, window_, gc_, indX, indY, indicatorSize, indicatorSize);
        if (selected_) {
            // Draw checkmark (X shape)
            XDrawLine(dpy, window_, gc_, indX + 2, indY + 2,
                      indX + indicatorSize - 2, indY + indicatorSize - 2);
            XDrawLine(dpy, window_, gc_, indX + indicatorSize - 2, indY + 2,
                      indX + 2, indY + indicatorSize - 2);
        }
    } else {
        // Diamond/circle radio button
        int cx = indX + indicatorSize / 2;
        int cy = indY + indicatorSize / 2;
        int r = indicatorSize / 2;
        XSetForeground(dpy, gc_, foreground_);
        XDrawArc(dpy, window_, gc_, cx - r, cy - r, 2 * r, 2 * r, 0, 360 * 64);
        if (selected_) {
            int fr = r - 3;
            if (fr > 0) {
                XFillArc(dpy, window_, gc_, cx - fr, cy - fr, 2 * fr, 2 * fr, 0, 360 * 64);
            }
        }
    }

    // Draw text to the right of indicator
    if (!text_.empty()) {
        XFontStruct* font = XQueryFont(dpy, XGContextFromGC(gc_));
        if (font) {
            int textX = indX + indicatorSize + marginWidth_;
            int ascent = font->ascent;
            int descent = font->descent;
            int textY = (height_ - (ascent + descent)) / 2 + ascent;

            XSetForeground(dpy, gc_, foreground_);
            XDrawString(dpy, window_, gc_, textX, textY,
                        text_.c_str(), static_cast<int>(text_.size()));
            XFreeFontInfo(nullptr, font, 0);
        }
    }

    if (focused_) drawHighlight();
}

void ToggleButton::handleButtonPress(unsigned int button, int /*x*/, int /*y*/) {
    if (button == 1 && sensitive_) {
        // Visual feedback handled on release
    }
}

void ToggleButton::handleButtonRelease(unsigned int button, int /*x*/, int /*y*/) {
    if (button == 1 && sensitive_) {
        setSelected(!selected_);
    }
}

} // namespace motif
