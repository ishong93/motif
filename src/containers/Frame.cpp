#include <motif/containers/Frame.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>

namespace motif {

Frame::Frame(Widget* parent) : Manager(parent) {
    shadowThickness_ = 2;
    resources_.set<int>("shadowType", static_cast<int>(ShadowType::EtchedIn));
    resources_.set<int>("marginWidth", 2);
    resources_.set<int>("marginHeight", 2);
}
Frame::~Frame() = default;

std::vector<ResourceSpec> Frame::resourceSpecs() const {
    auto specs = Manager::resourceSpecs();
    specs.emplace_back("shadowType", "ShadowType", typeid(int), 0,
                       ResourceValue{static_cast<int>(ShadowType::EtchedIn)});
    specs.emplace_back("marginWidth", "MarginWidth", typeid(int), 0, ResourceValue{2});
    specs.emplace_back("marginHeight", "MarginHeight", typeid(int), 0, ResourceValue{2});
    return specs;
}

void Frame::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "shadowType") {
        shadowType_ = static_cast<ShadowType>(resources_.get<int>("shadowType", 2));
    } else if (resourceName == "marginWidth") {
        marginWidth_ = resources_.get<int>("marginWidth", 2);
    } else if (resourceName == "marginHeight") {
        marginHeight_ = resources_.get<int>("marginHeight", 2);
    }
    Manager::onResourceChanged(resourceName);
}

void Frame::layout() {
    int inset = shadowThickness_ + marginWidth_;
    int insetV = shadowThickness_ + marginHeight_;
    for (auto* child : children_) {
        child->setPosition(inset, insetV,
                           width_ - 2 * inset,
                           height_ - 2 * insetV);
    }
}

void Frame::expose() {
    if (!window_ || !gc_) return;
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();

    XClearWindow(dpy, window_);

    bool raised = (shadowType_ == ShadowType::Out || shadowType_ == ShadowType::EtchedOut);
    unsigned long top = raised ? app.whitePixel() : app.blackPixel();
    unsigned long bot = raised ? app.blackPixel() : app.whitePixel();

    for (int i = 0; i < shadowThickness_; ++i) {
        XSetForeground(dpy, gc_, top);
        XDrawLine(dpy, window_, gc_, i, i, width_ - 1 - i, i);
        XDrawLine(dpy, window_, gc_, i, i, i, height_ - 1 - i);
        XSetForeground(dpy, gc_, bot);
        XDrawLine(dpy, window_, gc_, width_ - 1 - i, i, width_ - 1 - i, height_ - 1 - i);
        XDrawLine(dpy, window_, gc_, i, height_ - 1 - i, width_ - 1 - i, height_ - 1 - i);
    }
}

} // namespace motif
