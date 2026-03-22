#include <motif/widgets/PushButton.h>
#include <motif/core/Application.h>

#include <X11/Xlib.h>

namespace motif {

PushButton::PushButton(Widget* parent) : Label(parent) {
    shadowThickness_ = 2;
    resources_.set<bool>("showAsDefault", false);
    resources_.set<bool>("armed", false);
}

PushButton::~PushButton() = default;

void PushButton::setShowAsDefault(bool show) {
    showAsDefault_ = show;
    resources_.set<bool>("showAsDefault", show);
    if (realized_) handleExpose();
}

std::vector<ResourceSpec> PushButton::resourceSpecs() const {
    auto specs = Label::resourceSpecs();
    specs.emplace_back("showAsDefault", "ShowAsDefault", typeid(bool), 0,
                       ResourceValue{false});
    return specs;
}

void PushButton::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "showAsDefault") {
        showAsDefault_ = resources_.get<bool>("showAsDefault", false);
    }
    Label::onResourceChanged(resourceName);
}

void PushButton::arm() {
    armed_ = true;
    invokeCallbacks(ArmCallback);
    if (realized_) handleExpose();
}

void PushButton::disarm() {
    armed_ = false;
    invokeCallbacks(DisarmCallback);
    if (realized_) handleExpose();
}

void PushButton::activate() {
    invokeCallbacks(ActivateCallback);
}

void PushButton::expose() {
    if (!window_ || !gc_) return;
    auto* dpy = Application::instance().display();

    XClearWindow(dpy, window_);

    // Draw 3D shadow: raised when not armed, sunken when armed
    drawShadow(!armed_);

    // Draw text
    if (!text_.empty()) {
        XFontStruct* font = XQueryFont(dpy, XGContextFromGC(gc_));
        if (font) {
            int textWidth = XTextWidth(font, text_.c_str(), static_cast<int>(text_.size()));
            int ascent = font->ascent;
            int descent = font->descent;
            int textHeight = ascent + descent;

            int tx = (width_ - textWidth) / 2;
            int ty = (height_ - textHeight) / 2 + ascent;

            // Offset text when armed (pressed effect)
            if (armed_) {
                tx += 1;
                ty += 1;
            }

            XSetForeground(dpy, gc_, sensitive_ ? foreground_ : Application::instance().whitePixel());
            XDrawString(dpy, window_, gc_, tx, ty,
                        text_.c_str(), static_cast<int>(text_.size()));

            XFreeFontInfo(nullptr, font, 0);
        }
    }

    // Default button indicator
    if (showAsDefault_) {
        XSetForeground(dpy, gc_, foreground_);
        XDrawRectangle(dpy, window_, gc_, 0, 0, width_ - 1, height_ - 1);
    }

    if (focused_) {
        drawHighlight();
    }
}

void PushButton::handleButtonPress(unsigned int button, int /*x*/, int /*y*/) {
    if (button == 1 && sensitive_) {
        arm();
    }
}

void PushButton::handleButtonRelease(unsigned int button, int /*x*/, int /*y*/) {
    if (button == 1 && armed_) {
        disarm();
        if (pointerInside_) {
            activate();
        }
    }
}

void PushButton::handleEnter(int /*x*/, int /*y*/) {
    pointerInside_ = true;
}

void PushButton::handleLeave() {
    pointerInside_ = false;
    if (armed_) {
        disarm();
    }
}

} // namespace motif
