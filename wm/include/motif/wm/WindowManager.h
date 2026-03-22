#pragma once

#include <X11/Xlib.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

namespace motif::wm {

class Decoration;
class KeyFocus;
class Menu;
class EventHandler;
class EWMH;

// Managed client window state
struct ClientWindow {
    Window client = None;       // original client window
    Window frame = None;        // reparenting frame
    Window titleBar = None;     // title bar window
    Window closeBtn = None;     // close button
    Window maximizeBtn = None;  // maximize button
    Window minimizeBtn = None;  // minimize button

    std::string title;
    int x = 0, y = 0;
    int width = 0, height = 0;
    int borderWidth = 0;

    bool focused = false;
    bool maximized = false;
    bool iconified = false;
    bool hasDecorations = true;

    // Size hints from WM_NORMAL_HINTS
    int minWidth = 0, minHeight = 0;
    int maxWidth = 0, maxHeight = 0;
    int widthInc = 1, heightInc = 1;
    int baseWidth = 0, baseHeight = 0;

    // Saved geometry for maximize/restore
    int savedX = 0, savedY = 0;
    int savedWidth = 0, savedHeight = 0;

    // Virtual desktop (0xFFFFFFFF = sticky / visible on all desktops)
    int desktop = 0;
};

// Window manager configuration
struct WMConfig {
    int titleBarHeight = 24;
    int borderWidth = 4;
    int buttonSize = 18;

    unsigned long focusedTitleBg = 0;
    unsigned long unfocusedTitleBg = 0;
    unsigned long focusedTitleFg = 0;
    unsigned long unfocusedTitleFg = 0;
    unsigned long borderColor = 0;

    enum class FocusPolicy { ClickToFocus, FocusFollowsMouse };
    FocusPolicy focusPolicy = FocusPolicy::ClickToFocus;

    bool opaqueMove = true;
    bool opaqueResize = false;
    bool autoRaise = false;

    std::string fontName = "fixed";
};

class WindowManager {
public:
    WindowManager();
    ~WindowManager();

    bool initialize(const std::string& displayName = {});
    void run();
    void quit();

    // Client management
    void manageWindow(Window w);
    void unmanageWindow(Window w);
    ClientWindow* findClient(Window w);
    ClientWindow* findClientByFrame(Window w);

    // Window operations
    void focusClient(ClientWindow* client);
    void raiseClient(ClientWindow* client);
    void moveClient(ClientWindow* client, int x, int y);
    void resizeClient(ClientWindow* client, int w, int h);
    void maximizeClient(ClientWindow* client);
    void restoreClient(ClientWindow* client);
    void iconifyClient(ClientWindow* client);
    void closeClient(ClientWindow* client);

    // Virtual desktop management
    int numDesktops() const { return numDesktops_; }
    int currentDesktop() const { return currentDesktop_; }
    void setNumDesktops(int count);
    void switchDesktop(int desktop);
    void moveClientToDesktop(ClientWindow* client, int desktop);
    bool isClientOnCurrentDesktop(const ClientWindow* client) const;

    // Accessors
    Display* display() const { return display_; }
    int screen() const { return screen_; }
    Window root() const { return root_; }
    const WMConfig& config() const { return config_; }
    KeyFocus& keyFocus() { return *keyFocus_; }
    Decoration& decoration() { return *decoration_; }
    Menu& menu() { return *menu_; }
    EWMH& ewmh() { return *ewmh_; }

    const std::unordered_map<Window, std::unique_ptr<ClientWindow>>& clients() const {
        return clients_;
    }

private:
    void adoptExistingWindows();
    void setupAtoms();
    void initColors();
    void readClientProperties(ClientWindow* client);

    Display* display_ = nullptr;
    int screen_ = 0;
    Window root_ = None;
    bool running_ = false;

    int numDesktops_ = 4;
    int currentDesktop_ = 0;

    WMConfig config_;

    // WM protocol atoms
    Atom wmProtocols_ = None;
    Atom wmDeleteWindow_ = None;
    Atom wmTakeFocus_ = None;
    Atom wmState_ = None;
    Atom wmName_ = None;
    Atom netWmName_ = None;

    // Client windows indexed by client window ID
    std::unordered_map<Window, std::unique_ptr<ClientWindow>> clients_;
    // Frame window → client window mapping
    std::unordered_map<Window, Window> frameToClient_;

    std::unique_ptr<EventHandler> eventHandler_;
    std::unique_ptr<Decoration> decoration_;
    std::unique_ptr<KeyFocus> keyFocus_;
    std::unique_ptr<Menu> menu_;
    std::unique_ptr<EWMH> ewmh_;

    // X error handling
    static int xerrorHandler(Display* dpy, XErrorEvent* ev);
    static bool xerrorOccurred_;
};

} // namespace motif::wm
