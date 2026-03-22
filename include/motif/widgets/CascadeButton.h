#pragma once

#include <motif/widgets/Label.h>

namespace motif {

class PopupMenu;

class CascadeButton : public Label {
public:
    CascadeButton() = default;
    explicit CascadeButton(Widget* parent);
    ~CascadeButton() override;

    static constexpr const char* ActivateCallback  = "activate";
    static constexpr const char* CascadingCallback = "cascading";

    void setSubMenu(PopupMenu* menu) { subMenu_ = menu; }
    PopupMenu* subMenu() const { return subMenu_; }

    std::vector<ResourceSpec> resourceSpecs() const override;

    void handleButtonPress(unsigned int button, int x, int y) override;
    void handleButtonRelease(unsigned int button, int x, int y) override;

protected:
    void expose() override;

    PopupMenu* subMenu_ = nullptr;
};

} // namespace motif
