#pragma once

#include <motif/core/Primitive.h>
#include <string>

namespace motif {

class Scale : public Primitive {
public:
    Scale() = default;
    explicit Scale(Widget* parent);
    ~Scale() override;

    static constexpr const char* ValueChangedCallback = "valueChanged";
    static constexpr const char* DragCallback         = "drag";

    enum class Orientation { Horizontal, Vertical };

    void setOrientation(Orientation o) { orientation_ = o; }
    Orientation orientation() const { return orientation_; }

    void setValue(int value);
    int value() const { return value_; }

    void setMinimum(int min) { minimum_ = min; }
    int minimum() const { return minimum_; }

    void setMaximum(int max) { maximum_ = max; }
    int maximum() const { return maximum_; }

    void setShowValue(bool show) { showValue_ = show; }
    bool showValue() const { return showValue_; }

    void setTitle(const std::string& title) { title_ = title; }
    const std::string& title() const { return title_; }

    void setDecimalPoints(int dp) { decimalPoints_ = dp; }
    int decimalPoints() const { return decimalPoints_; }

    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void handleButtonPress(unsigned int button, int x, int y) override;
    void handleButtonRelease(unsigned int button, int x, int y) override;
    void handleMotionNotify(int x, int y, unsigned int state) override;

protected:
    void expose() override;

    int sliderPixelPos() const;
    int pixelToValue(int pixel) const;
    int troughStart() const;
    int troughLength() const;

    Orientation orientation_ = Orientation::Horizontal;
    int value_    = 0;
    int minimum_  = 0;
    int maximum_  = 100;
    bool showValue_    = true;
    int decimalPoints_ = 0;
    std::string title_;

    static constexpr int kSliderSize = 12;
    static constexpr int kTrackThickness = 6;
    static constexpr int kLabelArea = 20;

    bool dragging_ = false;
    int dragOffset_ = 0;
};

} // namespace motif
