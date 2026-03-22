#pragma once

#include <X11/Xlib.h>
#include <vector>

namespace motif::wm {

class WindowManager;
struct ClientWindow;

class KeyFocus {
public:
    explicit KeyFocus(WindowManager& wm);

    void setFocus(ClientWindow* client);
    ClientWindow* focusedClient() const { return focused_; }

    // Focus cycling
    void focusNext();
    void focusPrev();

    // Track focus stack (most recently focused first)
    void pushFocusStack(ClientWindow* client);
    void removeFocusStack(ClientWindow* client);

private:
    WindowManager& wm_;
    ClientWindow* focused_ = nullptr;
    std::vector<ClientWindow*> focusStack_;
};

} // namespace motif::wm
