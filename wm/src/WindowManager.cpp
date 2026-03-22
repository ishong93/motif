#include <motif/wm/WindowManager.h>
#include <motif/wm/EventHandler.h>
#include <motif/wm/Decoration.h>
#include <motif/wm/KeyFocus.h>
#include <motif/wm/Menu.h>
#include <motif/wm/EWMH.h>

#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include <iostream>
#include <algorithm>
#include <cstring>

namespace motif::wm {

bool WindowManager::xerrorOccurred_ = false;

int WindowManager::xerrorHandler(Display* /*dpy*/, XErrorEvent* /*ev*/) {
    xerrorOccurred_ = true;
    return 0;
}

WindowManager::WindowManager() = default;
WindowManager::~WindowManager() {
    if (display_) {
        XCloseDisplay(display_);
    }
}

bool WindowManager::initialize(const std::string& displayName) {
    const char* name = displayName.empty() ? nullptr : displayName.c_str();
    display_ = XOpenDisplay(name);
    if (!display_) {
        std::cerr << "motif-wm: cannot open display\n";
        return false;
    }

    screen_ = DefaultScreen(display_);
    root_ = RootWindow(display_, screen_);

    // Install our error handler
    XSetErrorHandler(xerrorHandler);

    // Try to select SubstructureRedirect on root — fails if another WM is running
    xerrorOccurred_ = false;
    XSelectInput(display_, root_,
                 SubstructureRedirectMask | SubstructureNotifyMask |
                 ButtonPressMask | KeyPressMask |
                 EnterWindowMask | PropertyChangeMask);
    XSync(display_, False);

    if (xerrorOccurred_) {
        std::cerr << "motif-wm: another window manager is already running\n";
        XCloseDisplay(display_);
        display_ = nullptr;
        return false;
    }

    setupAtoms();
    initColors();

    // Set root cursor
    Cursor cursor = XCreateFontCursor(display_, XC_left_ptr);
    XDefineCursor(display_, root_, cursor);
    XFreeCursor(display_, cursor);

    // Create subsystems
    decoration_ = std::make_unique<Decoration>(*this);
    decoration_->loadFont();
    keyFocus_ = std::make_unique<KeyFocus>(*this);
    menu_ = std::make_unique<Menu>(*this);
    menu_->buildRootMenu();
    menu_->buildWindowMenu();
    ewmh_ = std::make_unique<EWMH>(*this);
    ewmh_->initialize();
    eventHandler_ = std::make_unique<EventHandler>(*this);

    // Adopt existing windows
    adoptExistingWindows();

    // Grab Alt+Tab for window cycling
    XGrabKey(display_, XKeysymToKeycode(display_, XK_Tab),
             Mod1Mask, root_, True, GrabModeAsync, GrabModeAsync);
    // Grab Alt+F4 for close
    XGrabKey(display_, XKeysymToKeycode(display_, XK_F4),
             Mod1Mask, root_, True, GrabModeAsync, GrabModeAsync);

    return true;
}

void WindowManager::setupAtoms() {
    wmProtocols_ = XInternAtom(display_, "WM_PROTOCOLS", False);
    wmDeleteWindow_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
    wmTakeFocus_ = XInternAtom(display_, "WM_TAKE_FOCUS", False);
    wmState_ = XInternAtom(display_, "WM_STATE", False);
    wmName_ = XInternAtom(display_, "WM_NAME", False);
    netWmName_ = XInternAtom(display_, "_NET_WM_NAME", False);
}

void WindowManager::initColors() {
    auto black = BlackPixel(display_, screen_);
    auto white = WhitePixel(display_, screen_);

    Colormap cmap = DefaultColormap(display_, screen_);
    XColor color, exact;

    // Active title bar: steel blue
    if (XAllocNamedColor(display_, cmap, "SteelBlue", &color, &exact)) {
        config_.focusedTitleBg = color.pixel;
    } else {
        config_.focusedTitleBg = black;
    }
    config_.focusedTitleFg = white;

    // Inactive title bar: gray
    if (XAllocNamedColor(display_, cmap, "gray70", &color, &exact)) {
        config_.unfocusedTitleBg = color.pixel;
    } else {
        config_.unfocusedTitleBg = white;
    }
    config_.unfocusedTitleFg = black;

    // Border
    if (XAllocNamedColor(display_, cmap, "gray50", &color, &exact)) {
        config_.borderColor = color.pixel;
    } else {
        config_.borderColor = black;
    }
}

void WindowManager::adoptExistingWindows() {
    Window root_ret, parent_ret;
    Window* children = nullptr;
    unsigned int nChildren = 0;

    XQueryTree(display_, root_, &root_ret, &parent_ret, &children, &nChildren);

    for (unsigned int i = 0; i < nChildren; ++i) {
        XWindowAttributes attrs;
        if (XGetWindowAttributes(display_, children[i], &attrs) &&
            attrs.override_redirect == False &&
            attrs.map_state == IsViewable) {
            manageWindow(children[i]);
        }
    }

    if (children) XFree(children);
}

void WindowManager::run() {
    running_ = true;
    XEvent event;

    while (running_) {
        XNextEvent(display_, &event);
        eventHandler_->handleEvent(event);
    }
}

void WindowManager::quit() {
    running_ = false;
}

// ── Client management ────────────────────────────────────────

void WindowManager::manageWindow(Window w) {
    if (clients_.count(w)) return; // already managed

    XWindowAttributes attrs;
    if (!XGetWindowAttributes(display_, w, &attrs)) return;
    if (attrs.override_redirect) return;

    auto client = std::make_unique<ClientWindow>();
    client->client = w;
    client->x = attrs.x;
    client->y = attrs.y;
    client->width = attrs.width;
    client->height = attrs.height;
    client->borderWidth = attrs.border_width;

    readClientProperties(client.get());

    // Create decoration frame
    decoration_->createFrame(client.get());

    // Register mappings
    Window clientWin = client->client;
    frameToClient_[client->frame] = clientWin;
    frameToClient_[client->titleBar] = clientWin;
    if (client->closeBtn) frameToClient_[client->closeBtn] = clientWin;
    if (client->maximizeBtn) frameToClient_[client->maximizeBtn] = clientWin;
    if (client->minimizeBtn) frameToClient_[client->minimizeBtn] = clientWin;

    // Reparent client into frame
    XAddToSaveSet(display_, w);
    XSetWindowBorderWidth(display_, w, 0);
    XReparentWindow(display_, w, client->frame,
                    config_.borderWidth,
                    config_.titleBarHeight + config_.borderWidth);
    XMapWindow(display_, w);

    clients_[clientWin] = std::move(client);

    // Focus the new window
    focusClient(clients_[clientWin].get());

    keyFocus_->pushFocusStack(clients_[clientWin].get());

    clients_[clientWin]->desktop = currentDesktop_;
    ewmh_->setAllowedActions(clients_[clientWin].get());
    ewmh_->updateWindowDesktop(clients_[clientWin].get(), currentDesktop_);
    ewmh_->updateClientList();
}

void WindowManager::unmanageWindow(Window w) {
    auto it = clients_.find(w);
    if (it == clients_.end()) return;

    auto* client = it->second.get();

    keyFocus_->removeFocusStack(client);

    // Unregister frame mappings
    frameToClient_.erase(client->frame);
    frameToClient_.erase(client->titleBar);
    if (client->closeBtn) frameToClient_.erase(client->closeBtn);
    if (client->maximizeBtn) frameToClient_.erase(client->maximizeBtn);
    if (client->minimizeBtn) frameToClient_.erase(client->minimizeBtn);

    decoration_->destroyFrame(client);
    clients_.erase(it);

    ewmh_->updateClientList();

    // Refocus
    if (keyFocus_->focusedClient() == nullptr) {
        keyFocus_->focusNext();
    }
}

ClientWindow* WindowManager::findClient(Window w) {
    auto it = clients_.find(w);
    if (it != clients_.end()) return it->second.get();

    // Also check frame mapping
    auto fit = frameToClient_.find(w);
    if (fit != frameToClient_.end()) {
        auto cit = clients_.find(fit->second);
        if (cit != clients_.end()) return cit->second.get();
    }
    return nullptr;
}

ClientWindow* WindowManager::findClientByFrame(Window w) {
    auto fit = frameToClient_.find(w);
    if (fit != frameToClient_.end()) {
        auto cit = clients_.find(fit->second);
        if (cit != clients_.end()) return cit->second.get();
    }
    return nullptr;
}

void WindowManager::readClientProperties(ClientWindow* client) {
    // Read WM_NAME
    char* name = nullptr;
    if (XFetchName(display_, client->client, &name) && name) {
        client->title = name;
        XFree(name);
    } else {
        client->title = "(untitled)";
    }

    // Read WM_NORMAL_HINTS (size hints)
    XSizeHints hints;
    long supplied;
    if (XGetWMNormalHints(display_, client->client, &hints, &supplied)) {
        if (hints.flags & PMinSize) {
            client->minWidth = hints.min_width;
            client->minHeight = hints.min_height;
        }
        if (hints.flags & PMaxSize) {
            client->maxWidth = hints.max_width;
            client->maxHeight = hints.max_height;
        }
        if (hints.flags & PResizeInc) {
            client->widthInc = hints.width_inc;
            client->heightInc = hints.height_inc;
        }
        if (hints.flags & PBaseSize) {
            client->baseWidth = hints.base_width;
            client->baseHeight = hints.base_height;
        }
    }
}

// ── Window operations ────────────────────────────────────────

void WindowManager::focusClient(ClientWindow* client) {
    if (!client) return;

    // Unfocus previous
    auto* prev = keyFocus_->focusedClient();
    if (prev && prev != client) {
        prev->focused = false;
        decoration_->updateFocusDecoration(prev);
    }

    client->focused = true;
    keyFocus_->setFocus(client);
    decoration_->updateFocusDecoration(client);

    XSetInputFocus(display_, client->client, RevertToPointerRoot, CurrentTime);
    raiseClient(client);
    ewmh_->updateActiveWindow(client->client);
}

void WindowManager::raiseClient(ClientWindow* client) {
    if (!client || client->frame == None) return;
    XRaiseWindow(display_, client->frame);
}

void WindowManager::moveClient(ClientWindow* client, int x, int y) {
    if (!client) return;
    client->x = x;
    client->y = y;
    XMoveWindow(display_, client->frame, x, y);
}

void WindowManager::resizeClient(ClientWindow* client, int w, int h) {
    if (!client) return;

    // Apply size hints
    if (client->minWidth > 0) w = std::max(w, client->minWidth);
    if (client->minHeight > 0) h = std::max(h, client->minHeight);
    if (client->maxWidth > 0) w = std::min(w, client->maxWidth);
    if (client->maxHeight > 0) h = std::min(h, client->maxHeight);

    client->width = w;
    client->height = h;

    int frameW = w + 2 * config_.borderWidth;
    int frameH = h + config_.titleBarHeight + 2 * config_.borderWidth;

    XResizeWindow(display_, client->frame, frameW, frameH);
    XResizeWindow(display_, client->titleBar, frameW - 2 * config_.borderWidth,
                  config_.titleBarHeight);
    XResizeWindow(display_, client->client, w, h);

    decoration_->drawTitleBar(client);
}

void WindowManager::maximizeClient(ClientWindow* client) {
    if (!client || client->maximized) return;

    // Save current geometry
    client->savedX = client->x;
    client->savedY = client->y;
    client->savedWidth = client->width;
    client->savedHeight = client->height;

    int screenW = DisplayWidth(display_, screen_);
    int screenH = DisplayHeight(display_, screen_);

    client->maximized = true;
    moveClient(client, 0, 0);
    resizeClient(client, screenW - 2 * config_.borderWidth,
                 screenH - config_.titleBarHeight - 2 * config_.borderWidth);
}

void WindowManager::restoreClient(ClientWindow* client) {
    if (!client || !client->maximized) return;

    client->maximized = false;
    moveClient(client, client->savedX, client->savedY);
    resizeClient(client, client->savedWidth, client->savedHeight);
}

void WindowManager::iconifyClient(ClientWindow* client) {
    if (!client) return;
    client->iconified = true;
    XUnmapWindow(display_, client->frame);
}

void WindowManager::closeClient(ClientWindow* client) {
    if (!client) return;

    // Check if client supports WM_DELETE_WINDOW
    Atom* protocols = nullptr;
    int count = 0;
    bool supportsDelete = false;

    if (XGetWMProtocols(display_, client->client, &protocols, &count)) {
        for (int i = 0; i < count; ++i) {
            if (protocols[i] == wmDeleteWindow_) {
                supportsDelete = true;
                break;
            }
        }
        if (protocols) XFree(protocols);
    }

    if (supportsDelete) {
        XEvent ev;
        ev.type = ClientMessage;
        ev.xclient.window = client->client;
        ev.xclient.message_type = wmProtocols_;
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = static_cast<long>(wmDeleteWindow_);
        ev.xclient.data.l[1] = CurrentTime;
        XSendEvent(display_, client->client, False, NoEventMask, &ev);
    } else {
        XKillClient(display_, client->client);
    }
}

// ── Virtual desktop management ───────────────────────────────

void WindowManager::setNumDesktops(int count) {
    if (count < 1) count = 1;
    if (count > 32) count = 32;
    numDesktops_ = count;

    // Update EWMH root property
    long num = numDesktops_;
    XChangeProperty(display_, root_,
                    XInternAtom(display_, "_NET_NUMBER_OF_DESKTOPS", False),
                    XA_CARDINAL, 32, PropModeReplace,
                    reinterpret_cast<unsigned char*>(&num), 1);

    // If current desktop is beyond new count, switch down
    if (currentDesktop_ >= numDesktops_) {
        switchDesktop(numDesktops_ - 1);
    }

    ewmh_->updateWorkarea();
}

void WindowManager::switchDesktop(int desktop) {
    if (desktop < 0 || desktop >= numDesktops_) return;
    if (desktop == currentDesktop_) return;

    currentDesktop_ = desktop;

    // Update EWMH root property
    long cur = currentDesktop_;
    XChangeProperty(display_, root_,
                    XInternAtom(display_, "_NET_CURRENT_DESKTOP", False),
                    XA_CARDINAL, 32, PropModeReplace,
                    reinterpret_cast<unsigned char*>(&cur), 1);

    // Show/hide windows based on desktop assignment
    for (auto& [w, client] : clients_) {
        if (isClientOnCurrentDesktop(client.get())) {
            if (!client->iconified) {
                XMapWindow(display_, client->frame);
            }
        } else {
            XUnmapWindow(display_, client->frame);
        }
    }

    // Refocus
    keyFocus_->focusNext();
}

void WindowManager::moveClientToDesktop(ClientWindow* client, int desktop) {
    if (!client) return;
    // 0xFFFFFFFF means sticky (all desktops)
    if (desktop != static_cast<int>(0xFFFFFFFF) &&
        (desktop < 0 || desktop >= numDesktops_)) return;

    client->desktop = desktop;
    ewmh_->updateWindowDesktop(client, desktop);

    // Show/hide based on current desktop
    if (isClientOnCurrentDesktop(client)) {
        if (!client->iconified) {
            XMapWindow(display_, client->frame);
        }
    } else {
        XUnmapWindow(display_, client->frame);
    }
}

bool WindowManager::isClientOnCurrentDesktop(const ClientWindow* client) const {
    if (!client) return false;
    // Sticky windows (0xFFFFFFFF) are on all desktops
    if (client->desktop == static_cast<int>(0xFFFFFFFF)) return true;
    return client->desktop == currentDesktop_;
}

} // namespace motif::wm
