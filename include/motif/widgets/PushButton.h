#pragma once

#include <motif/widgets/Label.h>

namespace motif {

class PushButton : public Label {
public:
    PushButton() = default;
    explicit PushButton(Widget* parent);
    ~PushButton() override;

    static constexpr const char* ActivateCallback = "activate";
    static constexpr const char* ArmCallback = "arm";
    static constexpr const char* DisarmCallback = "disarm";

    void arm();
    void disarm();
    void activate();
    bool isArmed() const { return armed_; }

    void setShowAsDefault(bool show);
    bool showAsDefault() const { return showAsDefault_; }

    // Resource system
    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    // Event handlers
    void handleButtonPress(unsigned int button, int x, int y) override;
    void handleButtonRelease(unsigned int button, int x, int y) override;
    void handleEnter(int x, int y) override;
    void handleLeave() override;

protected:
    void expose() override;

    bool armed_ = false;
    bool showAsDefault_ = false;
    bool pointerInside_ = false;
};

} // namespace motif
