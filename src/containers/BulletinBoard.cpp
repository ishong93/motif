#include <motif/containers/BulletinBoard.h>

namespace motif {

BulletinBoard::BulletinBoard(Widget* parent) : Manager(parent) {
    resources_.set<int>("marginWidth", 10);
    resources_.set<int>("marginHeight", 10);
}
BulletinBoard::~BulletinBoard() = default;

std::vector<ResourceSpec> BulletinBoard::resourceSpecs() const {
    auto specs = Manager::resourceSpecs();
    specs.emplace_back("marginWidth", "MarginWidth", typeid(int), 0, ResourceValue{10});
    specs.emplace_back("marginHeight", "MarginHeight", typeid(int), 0, ResourceValue{10});
    return specs;
}

void BulletinBoard::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "marginWidth") {
        marginWidth_ = resources_.get<int>("marginWidth", 10);
    } else if (resourceName == "marginHeight") {
        marginHeight_ = resources_.get<int>("marginHeight", 10);
    }
    Manager::onResourceChanged(resourceName);
}

void BulletinBoard::layout() {
    // BulletinBoard: children are positioned absolutely by their x/y.
    // No automatic layout — just ensure children stay within margins.
}

} // namespace motif
