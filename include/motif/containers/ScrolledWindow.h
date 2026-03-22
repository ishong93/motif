#pragma once

#include <motif/core/Manager.h>

namespace motif {

class ScrollBar;

class ScrolledWindow : public Manager {
public:
    ScrolledWindow() = default;
    explicit ScrolledWindow(Widget* parent);
    ~ScrolledWindow() override;

    enum class ScrollingPolicy { Automatic, ApplicationDefined };
    enum class ScrollBarDisplayPolicy { Static, AsNeeded };

    void setScrollingPolicy(ScrollingPolicy policy) { scrollPolicy_ = policy; }
    ScrollingPolicy scrollingPolicy() const { return scrollPolicy_; }

    void setScrollBarDisplayPolicy(ScrollBarDisplayPolicy policy) { displayPolicy_ = policy; }
    ScrollBarDisplayPolicy scrollBarDisplayPolicy() const { return displayPolicy_; }

    void setWorkWindow(Widget* w) { workWindow_ = w; }
    Widget* workWindow() const { return workWindow_; }

    ScrollBar* horizontalScrollBar() const { return hScrollBar_; }
    ScrollBar* verticalScrollBar() const { return vScrollBar_; }

    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void layout() override;

protected:
    ScrollingPolicy scrollPolicy_ = ScrollingPolicy::Automatic;
    ScrollBarDisplayPolicy displayPolicy_ = ScrollBarDisplayPolicy::AsNeeded;
    Widget* workWindow_ = nullptr;
    ScrollBar* hScrollBar_ = nullptr;
    ScrollBar* vScrollBar_ = nullptr;
};

} // namespace motif
