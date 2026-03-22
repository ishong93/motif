#pragma once

#include <motif/menus/MenuShell.h>
#include <motif/containers/RowColumn.h>

namespace motif {

class PopupMenu {
public:
    PopupMenu() = default;
    explicit PopupMenu(Widget* parent);
    ~PopupMenu();

    void popup(int rootX, int rootY);
    void popdown();

    RowColumn& rowColumn() { return rc_; }
    MenuShell& shell() { return shell_; }

    void addChild(Widget* child);

private:
    MenuShell shell_;
    RowColumn rc_{&shell_};
};

} // namespace motif
