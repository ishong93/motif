#include <motif/core/Application.h>
#include <motif/core/Widget.h>
#include <motif/core/Log.h>
#include <motif/core/Error.h>
#include <motif/core/FontRenderer.h>
#include <motif/core/InputMethod.h>
#include <motif/clipboard/Transfer.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <unordered_map>
#include <stdexcept>

namespace motif {

struct Application::Impl {
    std::unordered_map<XWindow, Widget*> widgetMap;
};

Application& Application::instance() {
    static Application app;
    return app;
}

Application::~Application() {
    close();
}

bool Application::open(const std::string& displayName) {
    if (display_) return true;

    const char* name = displayName.empty() ? nullptr : displayName.c_str();
    display_ = XOpenDisplay(name);
    if (!display_) {
        MOTIF_LOG_ERROR << "Cannot open display: "
            << (name ? name : "$DISPLAY");
        return false;
    }

    screen_ = DefaultScreen(display_);
    impl_ = std::make_unique<Impl>();

    // Install X11 error handler
    X11ErrorHandler::instance().install();

    // Intern WM atoms
    wmDeleteWindow_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
    wmProtocols_ = XInternAtom(display_, "WM_PROTOCOLS", False);

    // Initialize transfer/clipboard system
    TransferManager::instance().initialize(display_);

    // Initialize font rendering system
    core::FontRenderer::instance().initialize(display_, screen_);

    // Initialize input method for internationalized text input
    core::InputMethod::instance().initialize(display_);

    MOTIF_LOG_INFO << "Display opened: " << DisplayString(display_);
    return true;
}

void Application::close() {
    if (display_) {
        core::InputMethod::instance().shutdown();
        core::FontRenderer::instance().shutdown();
        XCloseDisplay(display_);
        display_ = nullptr;
    }
    impl_.reset();
    running_ = false;
}

bool Application::isOpen() const {
    return display_ != nullptr;
}

XWindow Application::rootWindow() const {
    return RootWindow(display_, screen_);
}

int Application::depth() const {
    return DefaultDepth(display_, screen_);
}

unsigned long Application::blackPixel() const {
    return BlackPixel(display_, screen_);
}

unsigned long Application::whitePixel() const {
    return WhitePixel(display_, screen_);
}

void Application::registerWidget(XWindow window, Widget* widget) {
    if (impl_) {
        impl_->widgetMap[window] = widget;
    }
}

void Application::unregisterWidget(XWindow window) {
    if (impl_) {
        impl_->widgetMap.erase(window);
    }
}

Widget* Application::findWidget(XWindow window) const {
    if (!impl_) return nullptr;
    auto it = impl_->widgetMap.find(window);
    return (it != impl_->widgetMap.end()) ? it->second : nullptr;
}

void Application::run() {
    if (!display_) return;

    running_ = true;
    XEvent event;

    while (running_) {
        XNextEvent(display_, &event);
        dispatchEvent(event);
    }
}

void Application::quit() {
    running_ = false;
}

bool Application::processEvent() {
    if (!display_) return false;

    if (XPending(display_) > 0) {
        XEvent event;
        XNextEvent(display_, &event);
        dispatchEvent(event);
        return true;
    }
    return false;
}

void Application::dispatchEvent(XEvent_t& event) {
    XEvent& xev = reinterpret_cast<XEvent&>(event);

    // Let XIM filter events first (for input method composition)
    if (core::InputMethod::instance().filterEvent(xev)) {
        return;
    }

    // Find the widget that owns this window
    XWindow targetWindow = xev.xany.window;
    Widget* widget = findWidget(targetWindow);

    if (!widget) return;

    switch (xev.type) {
        case Expose:
            if (xev.xexpose.count == 0) {
                widget->handleExpose();
            }
            break;

        case ConfigureNotify:
            widget->handleConfigure(
                xev.xconfigure.x, xev.xconfigure.y,
                xev.xconfigure.width, xev.xconfigure.height
            );
            break;

        case MapNotify:
            widget->handleMap();
            break;

        case UnmapNotify:
            widget->handleUnmap();
            break;

        case ButtonPress:
            widget->handleButtonPress(
                xev.xbutton.button,
                xev.xbutton.x, xev.xbutton.y
            );
            break;

        case ButtonRelease:
            widget->handleButtonRelease(
                xev.xbutton.button,
                xev.xbutton.x, xev.xbutton.y
            );
            break;

        case MotionNotify:
            // Compress motion events: skip to the latest
            while (XPending(display_) > 0) {
                XEvent peek;
                XPeekEvent(display_, &peek);
                if (peek.type == MotionNotify && peek.xmotion.window == targetWindow) {
                    XNextEvent(display_, &xev);
                } else {
                    break;
                }
            }
            widget->handleMotionNotify(
                xev.xmotion.x, xev.xmotion.y,
                xev.xmotion.state
            );
            break;

        case EnterNotify:
            widget->handleEnter(xev.xcrossing.x, xev.xcrossing.y);
            break;

        case LeaveNotify:
            widget->handleLeave();
            break;

        case KeyPress:
            widget->handleKeyPress(xev.xkey.keycode, xev.xkey.state);
            break;

        case KeyRelease:
            widget->handleKeyRelease(xev.xkey.keycode, xev.xkey.state);
            break;

        case FocusIn:
            widget->handleFocusIn();
            break;

        case FocusOut:
            widget->handleFocusOut();
            break;

        case DestroyNotify:
            widget->handleDestroy();
            break;

        case ClientMessage:
            if (static_cast<XAtom>(xev.xclient.data.l[0]) == wmDeleteWindow_) {
                widget->handleClose();
            }
            break;

        case SelectionRequest:
            TransferManager::instance().handleSelectionRequest(xev.xselectionrequest);
            break;

        case SelectionNotify:
            TransferManager::instance().handleSelectionNotify(xev.xselection);
            break;

        case SelectionClear:
            TransferManager::instance().handleSelectionClear(xev.xselectionclear);
            break;

        default:
            break;
    }
}

} // namespace motif
