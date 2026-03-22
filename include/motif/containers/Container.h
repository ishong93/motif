#pragma once

#include <motif/core/Manager.h>
#include <string>
#include <vector>

namespace motif {

class Container : public Manager {
public:
    Container() = default;
    explicit Container(Widget* parent);
    ~Container() override;

    enum class LayoutType { Outline, Spatial, Detail };

    void setLayoutType(LayoutType type) { layoutType_ = type; }
    LayoutType layoutType() const { return layoutType_; }

    void setLargeIconSize(int w, int h) { largeW_ = w; largeH_ = h; }
    void setSmallIconSize(int w, int h) { smallW_ = w; smallH_ = h; }

    void setOutlineIndentation(int indent) { outlineIndent_ = indent; }
    int outlineIndentation() const { return outlineIndent_; }

    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void layout() override;

protected:
    LayoutType layoutType_ = LayoutType::Spatial;
    int largeW_ = 32, largeH_ = 32;
    int smallW_ = 16, smallH_ = 16;
    int outlineIndent_ = 20;
};

} // namespace motif
