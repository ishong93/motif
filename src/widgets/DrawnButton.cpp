#include <motif/widgets/DrawnButton.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>

namespace motif {

DrawnButton::DrawnButton(Widget* parent) : PushButton(parent) {
    resources_.set<int>("shadowType", static_cast<int>(ShadowType::Out));
    resources_.set<bool>("pushButtonEnabled", true);
}
DrawnButton::~DrawnButton() = default;

std::vector<ResourceSpec> DrawnButton::resourceSpecs() const {
    auto specs = PushButton::resourceSpecs();
    specs.emplace_back("shadowType", "ShadowType", typeid(int), 0,
                       ResourceValue{static_cast<int>(ShadowType::Out)});
    specs.emplace_back("pushButtonEnabled", "PushButtonEnabled", typeid(bool), 0,
                       ResourceValue{true});
    return specs;
}

void DrawnButton::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "shadowType") {
        shadowType_ = static_cast<ShadowType>(resources_.get<int>("shadowType", 3));
    } else if (resourceName == "pushButtonEnabled") {
        pushButtonEnabled_ = resources_.get<bool>("pushButtonEnabled", true);
    }
    PushButton::onResourceChanged(resourceName);
}

void DrawnButton::expose() {
    if (!window_ || !gc_) return;

    // Let application draw custom content via callback
    invokeCallbacks(ExposeCallback);

    // Draw shadow border
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();
    unsigned long topColor, bottomColor;

    bool raised = (shadowType_ == ShadowType::Out || shadowType_ == ShadowType::EtchedOut);
    topColor = raised ? app.whitePixel() : app.blackPixel();
    bottomColor = raised ? app.blackPixel() : app.whitePixel();

    XSetForeground(dpy, gc_, topColor);
    XDrawLine(dpy, window_, gc_, 0, 0, width_ - 1, 0);
    XDrawLine(dpy, window_, gc_, 0, 0, 0, height_ - 1);

    XSetForeground(dpy, gc_, bottomColor);
    XDrawLine(dpy, window_, gc_, width_ - 1, 0, width_ - 1, height_ - 1);
    XDrawLine(dpy, window_, gc_, 0, height_ - 1, width_ - 1, height_ - 1);
}

} // namespace motif
