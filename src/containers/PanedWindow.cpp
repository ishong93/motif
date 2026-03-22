#include <motif/containers/PanedWindow.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>
#include <algorithm>

namespace motif {

PanedWindow::PanedWindow(Widget* parent) : Manager(parent) {
    resources_.set<int>("orientation", static_cast<int>(Orientation::Vertical));
    resources_.set<int>("sashWidth", 10);
    resources_.set<int>("sashHeight", 8);
    resources_.set<int>("spacing", 2);
}
PanedWindow::~PanedWindow() = default;

std::vector<ResourceSpec> PanedWindow::resourceSpecs() const {
    auto specs = Manager::resourceSpecs();
    specs.emplace_back("orientation", "Orientation", typeid(int), 0,
                       ResourceValue{static_cast<int>(Orientation::Vertical)});
    specs.emplace_back("sashWidth", "SashWidth", typeid(int), 0, ResourceValue{10});
    specs.emplace_back("sashHeight", "SashHeight", typeid(int), 0, ResourceValue{8});
    specs.emplace_back("spacing", "Spacing", typeid(int), 0, ResourceValue{2});
    return specs;
}

void PanedWindow::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "orientation") {
        orientation_ = static_cast<Orientation>(resources_.get<int>("orientation", 1));
    } else if (resourceName == "sashWidth") {
        sashWidth_ = resources_.get<int>("sashWidth", 10);
    } else if (resourceName == "sashHeight") {
        sashHeight_ = resources_.get<int>("sashHeight", 8);
    } else if (resourceName == "spacing") {
        spacing_ = resources_.get<int>("spacing", 2);
    }
    Manager::onResourceChanged(resourceName);
}

void PanedWindow::layout() {
    if (children_.empty()) return;

    int n = static_cast<int>(children_.size());
    bool vert = (orientation_ == Orientation::Vertical);
    int totalSpace = vert ? height_ : width_;
    int sashTotal = (n - 1) * (vert ? sashHeight_ : sashWidth_) + (n - 1) * spacing_;
    int available = totalSpace - sashTotal;
    int paneSize = std::max(1, available / n);

    int pos = 0;
    panePositions_.clear();
    for (int i = 0; i < n; ++i) {
        if (vert) {
            children_[i]->setPosition(0, pos, width_, paneSize);
        } else {
            children_[i]->setPosition(pos, 0, paneSize, height_);
        }
        panePositions_.push_back(pos);
        pos += paneSize + (vert ? sashHeight_ : sashWidth_) + spacing_;
    }
}

void PanedWindow::expose() {
    if (!window_ || !gc_) return;
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();

    XClearWindow(dpy, window_);

    bool vert = (orientation_ == Orientation::Vertical);
    int n = static_cast<int>(children_.size());
    int paneSize = 0;
    if (n > 0) {
        int sashTotal = (n - 1) * (vert ? sashHeight_ : sashWidth_) + (n - 1) * spacing_;
        paneSize = std::max(1, ((vert ? height_ : width_) - sashTotal) / n);
    }

    // Draw sashes between panes
    for (int i = 0; i < n - 1; ++i) {
        int sashPos = (i + 1) * paneSize + i * ((vert ? sashHeight_ : sashWidth_) + spacing_);

        XSetForeground(dpy, gc_, app.whitePixel());
        if (vert) {
            XFillRectangle(dpy, window_, gc_, 0, sashPos, width_, sashHeight_);
            XSetForeground(dpy, gc_, app.blackPixel());
            XDrawRectangle(dpy, window_, gc_, 0, sashPos, width_ - 1, sashHeight_ - 1);
        } else {
            XFillRectangle(dpy, window_, gc_, sashPos, 0, sashWidth_, height_);
            XSetForeground(dpy, gc_, app.blackPixel());
            XDrawRectangle(dpy, window_, gc_, sashPos, 0, sashWidth_ - 1, height_ - 1);
        }
    }
}

} // namespace motif
