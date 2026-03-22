#pragma once

#include <X11/Xlib.h>
#include <string>
#include <vector>
#include <functional>

namespace motif::wm {

class WindowManager;

struct MenuItem {
    std::string label;
    std::function<void()> action;
    bool separator = false;
};

class Menu {
public:
    explicit Menu(WindowManager& wm);
    ~Menu();

    // Build the root (desktop right-click) menu
    void buildRootMenu();

    // Build the window (title bar) menu
    void buildWindowMenu();

    // Show/hide
    void showRootMenu(int x, int y);
    void showWindowMenu(Window client, int x, int y);
    void hide();

    bool isVisible() const { return menuWindow_ != None && visible_; }

    // Event handling
    void handleExpose();
    void handleButtonPress(int x, int y);
    void handleMotionNotify(int x, int y);

    Window menuWindow() const { return menuWindow_; }

private:
    void createMenuWindow();
    void drawMenu();
    int itemAtY(int y) const;

    WindowManager& wm_;
    Window menuWindow_ = None;
    GC gc_ = nullptr;
    XFontStruct* font_ = nullptr;

    std::vector<MenuItem> items_;
    int highlightedItem_ = -1;
    bool visible_ = false;
    Window targetClient_ = None;

    static constexpr int kItemHeight = 22;
    static constexpr int kMenuPadding = 4;
};

} // namespace motif::wm
