#include <motif/core/Manager.h>
#include <motif/core/Gadget.h>
#include <motif/core/Application.h>

#include <X11/Xlib.h>
#include <algorithm>

namespace motif {

Manager::Manager(Widget* parent) : Widget(parent) {
    auto& app = Application::instance();
    foreground_ = app.blackPixel();
    background_ = app.whitePixel();

    resources_.set<unsigned long>("foreground", foreground_);
    resources_.set<unsigned long>("background", background_);
    resources_.set<int>("shadowThickness", shadowThickness_);
}

Manager::~Manager() = default;

void Manager::setBackground(unsigned long pixel) {
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

std::vector<ResourceSpec> Manager::resourceSpecs() const {
    auto specs = Widget::resourceSpecs();
    specs.emplace_back("foreground", "Foreground", typeid(unsigned long), 0,
                       ResourceValue{0UL});
    specs.emplace_back("background", "Background", typeid(unsigned long), 0,
                       ResourceValue{~0UL});
    specs.emplace_back("shadowThickness", "ShadowThickness", typeid(int), 0,
                       ResourceValue{0});
    return specs;
}

void Manager::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "foreground") {
        foreground_ = resources_.get<unsigned long>("foreground", 0UL);
    } else if (resourceName == "background") {
        setBackground(resources_.get<unsigned long>("background", ~0UL));
        return;
    } else if (resourceName == "shadowThickness") {
        shadowThickness_ = resources_.get<int>("shadowThickness", 0);
    }
    Widget::onResourceChanged(resourceName);
}

void Manager::layout() {
    // Base: no-op. Subclasses implement specific layout logic.
}

void Manager::childChanged(Widget* /*child*/) {
    layout();
}

void Manager::handleConfigure(int x, int y, int w, int h) {
    Widget::handleConfigure(x, y, w, h);
    layout();
}

Widget* Manager::nextFocusChild(Widget* current) {
    if (children_.empty()) return nullptr;

    auto it = std::find(children_.begin(), children_.end(), current);
    if (it == children_.end() || ++it == children_.end()) {
        return children_.front();
    }
    return *it;
}

Widget* Manager::prevFocusChild(Widget* current) {
    if (children_.empty()) return nullptr;

    auto it = std::find(children_.begin(), children_.end(), current);
    if (it == children_.end() || it == children_.begin()) {
        return children_.back();
    }
    return *(--it);
}

void Manager::handleExpose() {
    expose();
    // Also expose all gadget children (they render into our window)
    for (auto* child : children_) {
        if (child->isGadget() && child->isVisible()) {
            child->handleExpose();
        }
    }
}

void Manager::handleButtonPress(unsigned int button, int x, int y) {
    Widget* gadget = gadgetAt(x, y);
    if (gadget) {
        gadget->handleButtonPress(button, x - gadget->x(), y - gadget->y());
    }
}

void Manager::handleButtonRelease(unsigned int button, int x, int y) {
    Widget* gadget = gadgetAt(x, y);
    if (gadget) {
        gadget->handleButtonRelease(button, x - gadget->x(), y - gadget->y());
    }
}

void Manager::handleEnter(int x, int y) {
    Widget* gadget = gadgetAt(x, y);
    if (gadget) {
        gadget->handleEnter(x - gadget->x(), y - gadget->y());
    }
}

void Manager::handleMotionNotify(int x, int y, unsigned int state) {
    Widget* gadget = gadgetAt(x, y);
    if (gadget) {
        gadget->handleMotionNotify(x - gadget->x(), y - gadget->y(), state);
    }
}

void Manager::handleLeave() {
    // Notify any gadget children that may need leave notification
    for (auto* child : children_) {
        if (child->isGadget()) {
            child->handleLeave();
        }
    }
}

Widget* Manager::gadgetAt(int x, int y) const {
    // Search in reverse order (last added = topmost)
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if ((*it)->isGadget()) {
            auto* gadget = static_cast<Gadget*>(*it);
            if (gadget->containsPoint(x, y)) {
                return gadget;
            }
        }
    }
    return nullptr;
}

void Manager::expose() {
    Widget::expose();
    drawShadow();
}

void Manager::drawShadow() {
    if (!window_ || !gc_ || shadowThickness_ <= 0) return;
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();

    for (int i = 0; i < shadowThickness_; ++i) {
        XSetForeground(dpy, gc_, app.whitePixel());
        XDrawLine(dpy, window_, gc_, i, i, width_ - 1 - i, i);
        XDrawLine(dpy, window_, gc_, i, i, i, height_ - 1 - i);

        XSetForeground(dpy, gc_, app.blackPixel());
        XDrawLine(dpy, window_, gc_, i, height_ - 1 - i, width_ - 1 - i, height_ - 1 - i);
        XDrawLine(dpy, window_, gc_, width_ - 1 - i, i, width_ - 1 - i, height_ - 1 - i);
    }
}

} // namespace motif
