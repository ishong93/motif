#pragma once

#include <motif/core/Manager.h>

namespace motif {

class RowColumn : public Manager {
public:
    RowColumn() = default;
    explicit RowColumn(Widget* parent);
    ~RowColumn() override;

    enum class Orientation { Horizontal, Vertical };
    enum class Packing { Tight, Column, None };

    void setOrientation(Orientation orient) { orientation_ = orient; }
    Orientation orientation() const { return orientation_; }

    void setPacking(Packing packing) { packing_ = packing; }
    Packing packing() const { return packing_; }

    void setNumColumns(int cols) { numColumns_ = cols; }
    int numColumns() const { return numColumns_; }

    void setSpacing(int spacing) { spacing_ = spacing; }
    int spacing() const { return spacing_; }

    void setMargin(int w, int h) { marginWidth_ = w; marginHeight_ = h; }

    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void layout() override;

protected:
    Orientation orientation_ = Orientation::Vertical;
    Packing packing_ = Packing::Tight;
    int numColumns_ = 1;
    int spacing_ = 4;
    int marginWidth_ = 4;
    int marginHeight_ = 4;
};

} // namespace motif
