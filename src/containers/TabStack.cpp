#include <motif/containers/TabStack.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>
#include <algorithm>

namespace motif {

TabStack::TabStack(Widget* parent) : Manager(parent) {
    resources_.set<int>("selectedTab", 0);
    resources_.set<int>("tabHeight", 26);
}
TabStack::~TabStack() = default;

std::vector<ResourceSpec> TabStack::resourceSpecs() const {
    auto specs = Manager::resourceSpecs();
    specs.emplace_back("selectedTab", "SelectedTab", typeid(int), 0, ResourceValue{0});
    specs.emplace_back("tabHeight", "TabHeight", typeid(int), 0, ResourceValue{26});
    return specs;
}

void TabStack::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "selectedTab") {
        int tab = resources_.get<int>("selectedTab", 0);
        if (tab != selectedTab_) setSelectedTab(tab);
        return;
    } else if (resourceName == "tabHeight") {
        tabHeight_ = resources_.get<int>("tabHeight", 26);
        layout();
    }
    Manager::onResourceChanged(resourceName);
}

void TabStack::addTab(Widget* child, const std::string& label) {
    tabs_.push_back({child, label});
    if (realized_) layout();
}

void TabStack::removeTab(int index) {
    if (index >= 0 && index < static_cast<int>(tabs_.size())) {
        tabs_.erase(tabs_.begin() + index);
        if (selectedTab_ >= static_cast<int>(tabs_.size())) {
            selectedTab_ = std::max(0, static_cast<int>(tabs_.size()) - 1);
        }
        if (realized_) layout();
    }
}

void TabStack::setSelectedTab(int index) {
    if (index >= 0 && index < static_cast<int>(tabs_.size())) {
        selectedTab_ = index;
        invokeCallbacks(TabSelectedCallback);
        layout();
        if (realized_) handleExpose();
    }
}

void TabStack::handleButtonPress(unsigned int button, int x, int y) {
    if (button != 1 || tabs_.empty()) return;

    if (y < tabHeight_) {
        int tabW = width_ / static_cast<int>(tabs_.size());
        int tabIndex = x / tabW;
        if (tabIndex >= 0 && tabIndex < static_cast<int>(tabs_.size())) {
            setSelectedTab(tabIndex);
        }
        return;
    }
    Manager::handleButtonPress(button, x, y);
}

void TabStack::layout() {
    for (size_t i = 0; i < tabs_.size(); ++i) {
        if (tabs_[i].child) {
            if (static_cast<int>(i) == selectedTab_) {
                tabs_[i].child->setPosition(0, tabHeight_, width_, height_ - tabHeight_);
                tabs_[i].child->show();
            } else {
                tabs_[i].child->hide();
            }
        }
    }
}

void TabStack::expose() {
    if (!window_ || !gc_) return;
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();

    XClearWindow(dpy, window_);

    int tabW = tabs_.empty() ? 0 : (width_ / static_cast<int>(tabs_.size()));

    for (size_t i = 0; i < tabs_.size(); ++i) {
        int tx = static_cast<int>(i) * tabW;
        bool active = (static_cast<int>(i) == selectedTab_);

        XSetForeground(dpy, gc_, active ? app.whitePixel() : app.blackPixel());
        XFillRectangle(dpy, window_, gc_, tx, 0, tabW, tabHeight_);

        XSetForeground(dpy, gc_, app.blackPixel());
        XDrawRectangle(dpy, window_, gc_, tx, 0, tabW - 1, tabHeight_ - 1);

        XSetForeground(dpy, gc_, active ? app.blackPixel() : app.whitePixel());
        XFontStruct* font = XQueryFont(dpy, XGContextFromGC(gc_));
        if (font) {
            int tw = XTextWidth(font, tabs_[i].label.c_str(),
                                static_cast<int>(tabs_[i].label.size()));
            XDrawString(dpy, window_, gc_,
                        tx + (tabW - tw) / 2,
                        font->ascent + 4,
                        tabs_[i].label.c_str(),
                        static_cast<int>(tabs_[i].label.size()));
            XFreeFontInfo(nullptr, font, 0);
        }
    }

    XSetForeground(dpy, gc_, app.blackPixel());
    XDrawRectangle(dpy, window_, gc_, 0, tabHeight_, width_ - 1, height_ - tabHeight_ - 1);
}

} // namespace motif
