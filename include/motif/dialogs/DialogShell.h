#pragma once

#include <motif/core/Shell.h>

namespace motif {

class DialogShell : public Shell {
public:
    DialogShell() = default;
    explicit DialogShell(Widget* parent);
    ~DialogShell() override;

    void setModal(bool modal) { modal_ = modal; }
    bool isModal() const { return modal_; }

    void popup();
    void popdown();

protected:
    void createWindow() override;

    bool modal_ = false;
};

} // namespace motif
