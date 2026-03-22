#pragma once

#include <motif/core/Shell.h>
#include <vector>

namespace motif {

class MenuShell : public Shell {
public:
    MenuShell() = default;
    explicit MenuShell(Widget* parent);
    ~MenuShell() override;

    void popup(int x, int y);
    void popdown();

    bool isPostedFromButton() const { return postedFromButton_; }

protected:
    void createWindow() override;

    bool postedFromButton_ = false;
};

} // namespace motif
