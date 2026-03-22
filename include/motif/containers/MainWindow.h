#pragma once

#include <motif/containers/ScrolledWindow.h>

namespace motif {

class MenuBar;

class MainWindow : public ScrolledWindow {
public:
    MainWindow() = default;
    explicit MainWindow(Widget* parent);
    ~MainWindow() override;

    void setMenuBar(Widget* menuBar) { menuBar_ = menuBar; }
    Widget* menuBar() const { return menuBar_; }

    void setCommandWindow(Widget* cmd) { commandWindow_ = cmd; }
    Widget* commandWindow() const { return commandWindow_; }

    void setMessageWindow(Widget* msg) { messageWindow_ = msg; }
    Widget* messageWindow() const { return messageWindow_; }

    std::vector<ResourceSpec> resourceSpecs() const override;

    void layout() override;

protected:
    Widget* menuBar_       = nullptr;
    Widget* commandWindow_ = nullptr;
    Widget* messageWindow_ = nullptr;
};

} // namespace motif
