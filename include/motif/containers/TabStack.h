#pragma once

#include <motif/core/Manager.h>
#include <string>
#include <vector>

namespace motif {

class TabStack : public Manager {
public:
    TabStack() = default;
    explicit TabStack(Widget* parent);
    ~TabStack() override;

    static constexpr const char* TabSelectedCallback = "tabSelected";

    struct Tab {
        Widget* child = nullptr;
        std::string label;
    };

    void addTab(Widget* child, const std::string& label);
    void removeTab(int index);

    void setSelectedTab(int index);
    int selectedTab() const { return selectedTab_; }
    int tabCount() const { return static_cast<int>(tabs_.size()); }

    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void layout() override;
    void handleButtonPress(unsigned int button, int x, int y) override;

protected:
    void expose() override;

    std::vector<Tab> tabs_;
    int selectedTab_ = 0;
    int tabHeight_ = 26;
};

} // namespace motif
