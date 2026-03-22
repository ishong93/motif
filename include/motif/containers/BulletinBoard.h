#pragma once

#include <motif/core/Manager.h>

namespace motif {

class BulletinBoard : public Manager {
public:
    BulletinBoard() = default;
    explicit BulletinBoard(Widget* parent);
    ~BulletinBoard() override;

    void setMarginWidth(int w) { marginWidth_ = w; }
    int marginWidth() const { return marginWidth_; }

    void setMarginHeight(int h) { marginHeight_ = h; }
    int marginHeight() const { return marginHeight_; }

    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void layout() override;

protected:
    int marginWidth_  = 10;
    int marginHeight_ = 10;
};

} // namespace motif
