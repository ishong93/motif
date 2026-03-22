#pragma once

#include <motif/widgets/Label.h>

namespace motif {

class ToggleButton : public Label {
public:
    ToggleButton() = default;
    explicit ToggleButton(Widget* parent);
    ~ToggleButton() override;

    static constexpr const char* ValueChangedCallback = "valueChanged";
    static constexpr const char* ArmCallback = "arm";
    static constexpr const char* DisarmCallback = "disarm";

    void setSelected(bool selected);
    bool isSelected() const { return selected_; }

    enum class IndicatorType { CheckBox, RadioButton };
    void setIndicatorType(IndicatorType type);
    IndicatorType indicatorType() const { return indicatorType_; }

    enum class ToggleMode { Boolean, Indeterminate };
    void setToggleMode(ToggleMode mode) { toggleMode_ = mode; }
    ToggleMode toggleMode() const { return toggleMode_; }

    // Resource system
    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void handleButtonPress(unsigned int button, int x, int y) override;
    void handleButtonRelease(unsigned int button, int x, int y) override;

protected:
    void expose() override;

    bool selected_ = false;
    IndicatorType indicatorType_ = IndicatorType::CheckBox;
    ToggleMode toggleMode_ = ToggleMode::Boolean;
};

} // namespace motif
