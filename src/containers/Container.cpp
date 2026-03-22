#include <motif/containers/Container.h>
#include <algorithm>

namespace motif {

Container::Container(Widget* parent) : Manager(parent) {
    resources_.set<int>("layoutType", static_cast<int>(LayoutType::Spatial));
    resources_.set<int>("outlineIndentation", 20);
}
Container::~Container() = default;

std::vector<ResourceSpec> Container::resourceSpecs() const {
    auto specs = Manager::resourceSpecs();
    specs.emplace_back("layoutType", "LayoutType", typeid(int), 0,
                       ResourceValue{static_cast<int>(LayoutType::Spatial)});
    specs.emplace_back("outlineIndentation", "OutlineIndentation", typeid(int), 0,
                       ResourceValue{20});
    return specs;
}

void Container::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "layoutType") {
        layoutType_ = static_cast<LayoutType>(resources_.get<int>("layoutType", 1));
        layout();
    } else if (resourceName == "outlineIndentation") {
        outlineIndent_ = resources_.get<int>("outlineIndentation", 20);
        layout();
    }
    Manager::onResourceChanged(resourceName);
}

void Container::layout() {
    if (children_.empty()) return;

    switch (layoutType_) {
        case LayoutType::Spatial: {
            // Grid-like arrangement
            int cols = std::max(1, width_ / (largeW_ + 8));
            int x = 4, y = 4;
            int col = 0;
            for (auto* child : children_) {
                child->setPosition(x, y, largeW_, largeH_);
                x += largeW_ + 8;
                if (++col >= cols) {
                    col = 0;
                    x = 4;
                    y += largeH_ + 8;
                }
            }
            break;
        }
        case LayoutType::Outline: {
            int y = 4;
            for (auto* child : children_) {
                child->setPosition(outlineIndent_, y, width_ - outlineIndent_ - 4, smallH_);
                y += smallH_ + 2;
            }
            break;
        }
        case LayoutType::Detail: {
            int y = 4;
            for (auto* child : children_) {
                child->setPosition(4, y, width_ - 8, smallH_);
                y += smallH_ + 2;
            }
            break;
        }
    }
}

} // namespace motif
