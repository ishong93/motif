#pragma once

#include <motif/core/Primitive.h>

namespace motif {

class ScrollBar : public Primitive {
public:
    ScrollBar() = default;
    explicit ScrollBar(Widget* parent);
    ~ScrollBar() override;

    static constexpr const char* ValueChangedCallback  = "valueChanged";
    static constexpr const char* IncrementCallback     = "increment";
    static constexpr const char* DecrementCallback     = "decrement";
    static constexpr const char* PageIncrementCallback = "pageIncrement";
    static constexpr const char* PageDecrementCallback = "pageDecrement";
    static constexpr const char* DragCallback          = "drag";

    enum class Orientation { Horizontal, Vertical };

    void setOrientation(Orientation o) { orientation_ = o; }
    Orientation orientation() const { return orientation_; }

    void setValue(int value);
    int value() const { return value_; }

    void setMinimum(int min) { minimum_ = min; }
    int minimum() const { return minimum_; }

    void setMaximum(int max) { maximum_ = max; }
    int maximum() const { return maximum_; }

    void setSliderSize(int size) { sliderSize_ = size; }
    int sliderSize() const { return sliderSize_; }

    void setIncrement(int inc) { increment_ = inc; }
    int increment() const { return increment_; }

    void setPageIncrement(int inc) { pageIncrement_ = inc; }
    int pageIncrement() const { return pageIncrement_; }

    // Event handlers
    // Resource system
    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void handleButtonPress(unsigned int button, int x, int y) override;
    void handleButtonRelease(unsigned int button, int x, int y) override;
    void handleMotionNotify(int x, int y, unsigned int state) override;

protected:
    void expose() override;

    // Geometry helpers
    int arrowSize() const;
    int troughLength() const;
    int sliderPixelSize() const;
    int sliderPixelPos() const;
    int pixelToValue(int pixel) const;

    enum class HitZone { None, Arrow1, Arrow2, SliderBefore, Slider, SliderAfter };
    HitZone hitTest(int x, int y) const;

    void drawArrow(int ax, int ay, int size, bool up);

    Orientation orientation_ = Orientation::Vertical;
    int value_       = 0;
    int minimum_     = 0;
    int maximum_     = 100;
    int sliderSize_  = 10;
    int increment_   = 1;
    int pageIncrement_ = 10;

    bool dragging_   = false;
    int dragOffset_  = 0;
};

} // namespace motif
