#include <motif/containers/ScrolledWindow.h>
#include <motif/widgets/ScrollBar.h>
#include <algorithm>

namespace motif {

ScrolledWindow::ScrolledWindow(Widget* parent) : Manager(parent) {
    resources_.set<int>("scrollingPolicy", static_cast<int>(ScrollingPolicy::Automatic));
    resources_.set<int>("scrollBarDisplayPolicy", static_cast<int>(ScrollBarDisplayPolicy::AsNeeded));
}
ScrolledWindow::~ScrolledWindow() = default;

std::vector<ResourceSpec> ScrolledWindow::resourceSpecs() const {
    auto specs = Manager::resourceSpecs();
    specs.emplace_back("scrollingPolicy", "ScrollingPolicy", typeid(int), 0,
                       ResourceValue{static_cast<int>(ScrollingPolicy::Automatic)});
    specs.emplace_back("scrollBarDisplayPolicy", "ScrollBarDisplayPolicy", typeid(int), 0,
                       ResourceValue{static_cast<int>(ScrollBarDisplayPolicy::AsNeeded)});
    return specs;
}

void ScrolledWindow::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "scrollingPolicy") {
        scrollPolicy_ = static_cast<ScrollingPolicy>(resources_.get<int>("scrollingPolicy", 0));
    } else if (resourceName == "scrollBarDisplayPolicy") {
        displayPolicy_ = static_cast<ScrollBarDisplayPolicy>(resources_.get<int>("scrollBarDisplayPolicy", 1));
    }
    Manager::onResourceChanged(resourceName);
}

void ScrolledWindow::layout() {
    int scrollBarWidth = 16;
    bool needH = false, needV = false;

    if (workWindow_) {
        if (displayPolicy_ == ScrollBarDisplayPolicy::Static) {
            needH = needV = true;
        } else {
            needH = workWindow_->width() > width_;
            needV = workWindow_->height() > height_;
        }
    }

    int viewW = width_ - (needV ? scrollBarWidth : 0);
    int viewH = height_ - (needH ? scrollBarWidth : 0);

    if (workWindow_ && scrollPolicy_ == ScrollingPolicy::Automatic) {
        workWindow_->setPosition(0, 0, std::max(viewW, workWindow_->width()),
                                       std::max(viewH, workWindow_->height()));
    }

    if (vScrollBar_ && needV) {
        vScrollBar_->setPosition(viewW, 0, scrollBarWidth, viewH);
        vScrollBar_->show();
    } else if (vScrollBar_) {
        vScrollBar_->hide();
    }

    if (hScrollBar_ && needH) {
        hScrollBar_->setPosition(0, viewH, viewW, scrollBarWidth);
        hScrollBar_->show();
    } else if (hScrollBar_) {
        hScrollBar_->hide();
    }
}

} // namespace motif
