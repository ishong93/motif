#pragma once

#include <motif/core/Manager.h>
#include <vector>

namespace motif {

class PanedWindow : public Manager {
public:
    PanedWindow() = default;
    explicit PanedWindow(Widget* parent);
    ~PanedWindow() override;

    enum class Orientation { Horizontal, Vertical };

    void setOrientation(Orientation o) { orientation_ = o; }
    Orientation orientation() const { return orientation_; }

    void setSashWidth(int w) { sashWidth_ = w; }
    int sashWidth() const { return sashWidth_; }

    void setSashHeight(int h) { sashHeight_ = h; }
    int sashHeight() const { return sashHeight_; }

    void setSpacing(int s) { spacing_ = s; }
    int spacing() const { return spacing_; }

    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void layout() override;

protected:
    void expose() override;

    Orientation orientation_ = Orientation::Vertical;
    int sashWidth_  = 10;
    int sashHeight_ = 8;
    int spacing_    = 2;
    std::vector<int> panePositions_;
};

} // namespace motif
