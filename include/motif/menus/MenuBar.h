#pragma once

#include <motif/core/Manager.h>

namespace motif {

class MenuBar : public Manager {
public:
    MenuBar() = default;
    explicit MenuBar(Widget* parent);
    ~MenuBar() override;

    void setHelpWidget(Widget* w) { helpWidget_ = w; }
    Widget* helpWidget() const { return helpWidget_; }

    void layout() override;

protected:
    void expose() override;

    Widget* helpWidget_ = nullptr;
};

} // namespace motif
