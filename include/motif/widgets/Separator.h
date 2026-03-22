#pragma once

#include <motif/core/Primitive.h>

namespace motif {

class Separator : public Primitive {
public:
    Separator() = default;
    explicit Separator(Widget* parent);
    ~Separator() override;

    enum class Type { SingleLine, DoubleLine, SingleDashed, DoubleDashed,
                      NoLine, ShadowEtchedIn, ShadowEtchedOut };
    enum class Orientation { Horizontal, Vertical };

    void setSeparatorType(Type type) { type_ = type; }
    Type separatorType() const { return type_; }

    void setOrientation(Orientation o) { orientation_ = o; }
    Orientation orientation() const { return orientation_; }

    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

protected:
    void expose() override;

    Type type_ = Type::ShadowEtchedIn;
    Orientation orientation_ = Orientation::Horizontal;
};

} // namespace motif
