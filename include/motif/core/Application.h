#pragma once

#include <memory>
#include <string>

// Forward-declare X11 types to avoid leaking Xlib headers into public API
using XDisplay = struct _XDisplay;
using XWindow = unsigned long;
using XAtom = unsigned long;
using XGC = struct _XGC*;
using XEvent_t = union _XEvent;

namespace motif {

class Widget;

class Application {
public:
    static Application& instance();

    bool open(const std::string& displayName = {});
    void close();
    bool isOpen() const;

    // Event loop
    void run();
    void quit();
    bool isRunning() const { return running_; }

    // Process a single pending event (non-blocking); returns false if none
    bool processEvent();

    // X11 accessors
    XDisplay* display() const { return display_; }
    int screen() const { return screen_; }
    XWindow rootWindow() const;
    int depth() const;
    unsigned long blackPixel() const;
    unsigned long whitePixel() const;

    // Widget registry (maps X Window → Widget for event dispatch)
    void registerWidget(XWindow window, Widget* widget);
    void unregisterWidget(XWindow window);
    Widget* findWidget(XWindow window) const;

    // WM Atoms
    XAtom wmDeleteWindow() const { return wmDeleteWindow_; }
    XAtom wmProtocols() const { return wmProtocols_; }

private:
    Application() = default;
    ~Application();
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void dispatchEvent(XEvent_t& event);

    XDisplay* display_ = nullptr;
    int screen_ = 0;
    bool running_ = false;

    XAtom wmDeleteWindow_ = 0;
    XAtom wmProtocols_ = 0;

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace motif
