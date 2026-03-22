#include <motif/widgets/Separator.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>

namespace motif {

Separator::Separator(Widget* parent) : Primitive(parent) {
    highlightThickness_ = 0;
    resources_.set<int>("separatorType", static_cast<int>(Type::ShadowEtchedIn));
    resources_.set<int>("orientation", static_cast<int>(Orientation::Horizontal));
}
Separator::~Separator() = default;

std::vector<ResourceSpec> Separator::resourceSpecs() const {
    auto specs = Primitive::resourceSpecs();
    specs.emplace_back("separatorType", "SeparatorType", typeid(int), 0,
                       ResourceValue{static_cast<int>(Type::ShadowEtchedIn)});
    specs.emplace_back("orientation", "Orientation", typeid(int), 0,
                       ResourceValue{static_cast<int>(Orientation::Horizontal)});
    return specs;
}

void Separator::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "separatorType") {
        type_ = static_cast<Type>(resources_.get<int>("separatorType", 5));
    } else if (resourceName == "orientation") {
        orientation_ = static_cast<Orientation>(resources_.get<int>("orientation", 0));
    }
    Primitive::onResourceChanged(resourceName);
}

void Separator::expose() {
    if (!window_ || !gc_) return;
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();

    XClearWindow(dpy, window_);

    bool horiz = (orientation_ == Orientation::Horizontal);
    int mid = horiz ? height_ / 2 : width_ / 2;

    switch (type_) {
        case Type::SingleLine:
            XSetForeground(dpy, gc_, app.blackPixel());
            if (horiz) XDrawLine(dpy, window_, gc_, 0, mid, width_, mid);
            else       XDrawLine(dpy, window_, gc_, mid, 0, mid, height_);
            break;
        case Type::DoubleLine:
            XSetForeground(dpy, gc_, app.blackPixel());
            if (horiz) {
                XDrawLine(dpy, window_, gc_, 0, mid - 1, width_, mid - 1);
                XDrawLine(dpy, window_, gc_, 0, mid + 1, width_, mid + 1);
            } else {
                XDrawLine(dpy, window_, gc_, mid - 1, 0, mid - 1, height_);
                XDrawLine(dpy, window_, gc_, mid + 1, 0, mid + 1, height_);
            }
            break;
        case Type::ShadowEtchedIn:
        case Type::ShadowEtchedOut: {
            bool raised = (type_ == Type::ShadowEtchedOut);
            unsigned long top = raised ? app.whitePixel() : app.blackPixel();
            unsigned long bot = raised ? app.blackPixel() : app.whitePixel();
            if (horiz) {
                XSetForeground(dpy, gc_, top);
                XDrawLine(dpy, window_, gc_, 0, mid - 1, width_, mid - 1);
                XSetForeground(dpy, gc_, bot);
                XDrawLine(dpy, window_, gc_, 0, mid, width_, mid);
            } else {
                XSetForeground(dpy, gc_, top);
                XDrawLine(dpy, window_, gc_, mid - 1, 0, mid - 1, height_);
                XSetForeground(dpy, gc_, bot);
                XDrawLine(dpy, window_, gc_, mid, 0, mid, height_);
            }
            break;
        }
        case Type::NoLine:
        default:
            break;
    }
}

} // namespace motif
