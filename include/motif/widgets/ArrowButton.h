#pragma once

#include <motif/core/Primitive.h>

namespace motif {

class ArrowButton : public Primitive {
public:
    enum class Direction { Up, Down, Left, Right };

    ArrowButton() = default;
    explicit ArrowButton(Widget* parent);
    ~ArrowButton() override;

    static constexpr const char* ActivateCallback = "activate";
    static constexpr const char* ArmCallback      = "arm";
    static constexpr const char* DisarmCallback    = "disarm";

    void setDirection(Direction dir) { direction_ = dir; }
    Direction direction() const { return direction_; }

    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void handleButtonPress(unsigned int button, int x, int y) override;
    void handleButtonRelease(unsigned int button, int x, int y) override;

protected:
    void expose() override;

    Direction direction_ = Direction::Up;
    bool armed_ = false;
};

} // namespace motif
