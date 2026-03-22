#pragma once

#include <motif/widgets/PushButton.h>
#include <functional>

namespace motif {

class DrawnButton : public PushButton {
public:
    DrawnButton() = default;
    explicit DrawnButton(Widget* parent);
    ~DrawnButton() override;

    static constexpr const char* ExposeCallback = "expose";
    static constexpr const char* ResizeCallback = "resize";

    enum class ShadowType { In, Out, EtchedIn, EtchedOut };

    void setShadowType(ShadowType type) { shadowType_ = type; }
    ShadowType shadowType() const { return shadowType_; }

    void setPushButtonEnabled(bool enabled) { pushButtonEnabled_ = enabled; }
    bool pushButtonEnabled() const { return pushButtonEnabled_; }

    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

protected:
    void expose() override;

    ShadowType shadowType_ = ShadowType::Out;
    bool pushButtonEnabled_ = true;
};

} // namespace motif
