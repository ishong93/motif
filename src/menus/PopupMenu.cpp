#include <motif/menus/PopupMenu.h>

namespace motif {

PopupMenu::PopupMenu(Widget* parent) : shell_(parent) {
    rc_.setOrientation(RowColumn::Orientation::Vertical);
}

PopupMenu::~PopupMenu() = default;

void PopupMenu::addChild(Widget* child) {
    rc_.addChild(child);
}

void PopupMenu::popup(int rootX, int rootY) {
    // Resize shell to fit content
    int itemH = 24;
    int n = static_cast<int>(rc_.children().size());
    shell_.resize(160, std::max(20, n * itemH));
    shell_.popup(rootX, rootY);
    rc_.layout();
}

void PopupMenu::popdown() {
    shell_.popdown();
}

} // namespace motif
