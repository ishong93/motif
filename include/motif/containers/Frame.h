#pragma once

#include <motif/core/Manager.h>

namespace motif {

class Frame : public Manager {
public:
    Frame() = default;
    explicit Frame(Widget* parent);
    ~Frame() override;

    enum class ShadowType { In, Out, EtchedIn, EtchedOut };

    void setShadowType(ShadowType type) { shadowType_ = type; }
    ShadowType shadowType() const { return shadowType_; }

    void setMarginWidth(int w) { marginWidth_ = w; }
    void setMarginHeight(int h) { marginHeight_ = h; }

    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void layout() override;

protected:
    void expose() override;

    ShadowType shadowType_ = ShadowType::EtchedIn;
    int marginWidth_  = 2;
    int marginHeight_ = 2;
};

} // namespace motif
