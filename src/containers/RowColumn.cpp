#include <motif/containers/RowColumn.h>
#include <algorithm>

namespace motif {

RowColumn::RowColumn(Widget* parent) : Manager(parent) {
    resources_.set<int>("orientation", static_cast<int>(Orientation::Vertical));
    resources_.set<int>("packing", static_cast<int>(Packing::Tight));
    resources_.set<int>("numColumns", 1);
    resources_.set<int>("spacing", 4);
    resources_.set<int>("marginWidth", 4);
    resources_.set<int>("marginHeight", 4);
}
RowColumn::~RowColumn() = default;

std::vector<ResourceSpec> RowColumn::resourceSpecs() const {
    auto specs = Manager::resourceSpecs();
    specs.emplace_back("orientation", "Orientation", typeid(int), 0,
                       ResourceValue{static_cast<int>(Orientation::Vertical)});
    specs.emplace_back("packing", "Packing", typeid(int), 0,
                       ResourceValue{static_cast<int>(Packing::Tight)});
    specs.emplace_back("numColumns", "NumColumns", typeid(int), 0, ResourceValue{1});
    specs.emplace_back("spacing", "Spacing", typeid(int), 0, ResourceValue{4});
    specs.emplace_back("marginWidth", "MarginWidth", typeid(int), 0, ResourceValue{4});
    specs.emplace_back("marginHeight", "MarginHeight", typeid(int), 0, ResourceValue{4});
    return specs;
}

void RowColumn::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "orientation") {
        orientation_ = static_cast<Orientation>(resources_.get<int>("orientation", 1));
    } else if (resourceName == "packing") {
        packing_ = static_cast<Packing>(resources_.get<int>("packing", 0));
    } else if (resourceName == "numColumns") {
        numColumns_ = resources_.get<int>("numColumns", 1);
    } else if (resourceName == "spacing") {
        spacing_ = resources_.get<int>("spacing", 4);
    } else if (resourceName == "marginWidth") {
        marginWidth_ = resources_.get<int>("marginWidth", 4);
    } else if (resourceName == "marginHeight") {
        marginHeight_ = resources_.get<int>("marginHeight", 4);
    }
    Manager::onResourceChanged(resourceName);
}

void RowColumn::layout() {
    if (children_.empty()) return;

    int numChildren = static_cast<int>(children_.size());
    int availWidth = width_ - 2 * marginWidth_;
    int availHeight = height_ - 2 * marginHeight_;

    if (orientation_ == Orientation::Vertical) {
        // Divide available height equally among children
        int totalSpacing = spacing_ * (numChildren - 1);
        int childHeight = (numChildren > 0)
            ? (availHeight - totalSpacing) / numChildren : 0;
        childHeight = std::max(childHeight, 1);

        int yPos = marginHeight_;
        for (auto* child : children_) {
            child->setPosition(marginWidth_, yPos, availWidth, childHeight);
            yPos += childHeight + spacing_;
        }
    } else {
        // Horizontal: divide width equally
        int totalSpacing = spacing_ * (numChildren - 1);
        int childWidth = (numChildren > 0)
            ? (availWidth - totalSpacing) / numChildren : 0;
        childWidth = std::max(childWidth, 1);

        int xPos = marginWidth_;
        for (auto* child : children_) {
            child->setPosition(xPos, marginHeight_, childWidth, availHeight);
            xPos += childWidth + spacing_;
        }
    }
}

} // namespace motif
