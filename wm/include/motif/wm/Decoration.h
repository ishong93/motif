#pragma once

#include <X11/Xlib.h>
#include <string>

namespace motif::wm {

class WindowManager;
struct ClientWindow;

class Decoration {
public:
    explicit Decoration(WindowManager& wm);
    ~Decoration();

    // Create frame and decoration windows for a client
    void createFrame(ClientWindow* client);
    void destroyFrame(ClientWindow* client);

    // Drawing
    void drawTitleBar(ClientWindow* client);
    void drawBorder(ClientWindow* client);
    void drawCloseButton(ClientWindow* client, bool pressed = false);
    void drawMaximizeButton(ClientWindow* client, bool pressed = false);
    void drawMinimizeButton(ClientWindow* client, bool pressed = false);

    // Update focus visual
    void updateFocusDecoration(ClientWindow* client);

    // Load font
    void loadFont();

private:
    WindowManager& wm_;
    GC gc_ = nullptr;
    XFontStruct* font_ = nullptr;
};

} // namespace motif::wm
