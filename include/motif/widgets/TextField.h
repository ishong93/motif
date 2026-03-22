#pragma once

#include <motif/widgets/Text.h>

namespace motif {

// Single-line text input widget (convenience subclass of Text).
class TextField : public Text {
public:
    TextField() = default;
    explicit TextField(Widget* parent);
    ~TextField() override;

    static constexpr const char* ActivateCallback = "activate";

    std::vector<ResourceSpec> resourceSpecs() const override;

    void handleKeyPress(unsigned int keycode, unsigned int state) override;

protected:
    void expose() override;
};

} // namespace motif
