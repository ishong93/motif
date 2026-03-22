#pragma once

#include <X11/Xlib.h>

namespace motif::wm {

class WindowManager;

class EventHandler {
public:
    explicit EventHandler(WindowManager& wm);

    void handleEvent(XEvent& event);

private:
    void handleMapRequest(XMapRequestEvent& ev);
    void handleUnmapNotify(XUnmapEvent& ev);
    void handleDestroyNotify(XDestroyWindowEvent& ev);
    void handleConfigureRequest(XConfigureRequestEvent& ev);
    void handleButtonPress(XButtonEvent& ev);
    void handleButtonRelease(XButtonEvent& ev);
    void handleMotionNotify(XMotionEvent& ev);
    void handleEnterNotify(XCrossingEvent& ev);
    void handleExpose(XExposeEvent& ev);
    void handlePropertyNotify(XPropertyEvent& ev);
    void handleClientMessage(XClientMessageEvent& ev);
    void handleKeyPress(XKeyEvent& ev);

    WindowManager& wm_;

    // Drag state for move/resize
    enum class DragMode { None, Moving, Resizing };
    DragMode dragMode_ = DragMode::None;
    Window dragWindow_ = None;
    int dragStartX_ = 0, dragStartY_ = 0;
    int dragOrigX_ = 0, dragOrigY_ = 0;
    int dragOrigW_ = 0, dragOrigH_ = 0;
};

} // namespace motif::wm
