#include <motif/wm/KeyFocus.h>
#include <motif/wm/WindowManager.h>

#include <algorithm>

namespace motif::wm {

KeyFocus::KeyFocus(WindowManager& wm) : wm_(wm) {}

void KeyFocus::setFocus(ClientWindow* client) {
    focused_ = client;
}

void KeyFocus::focusNext() {
    if (focusStack_.empty()) return;

    if (!focused_) {
        wm_.focusClient(focusStack_.front());
        return;
    }

    // Find current in stack
    auto it = std::find(focusStack_.begin(), focusStack_.end(), focused_);
    if (it == focusStack_.end() || ++it == focusStack_.end()) {
        // Wrap around
        wm_.focusClient(focusStack_.front());
    } else {
        wm_.focusClient(*it);
    }
}

void KeyFocus::focusPrev() {
    if (focusStack_.empty()) return;

    if (!focused_) {
        wm_.focusClient(focusStack_.back());
        return;
    }

    auto it = std::find(focusStack_.begin(), focusStack_.end(), focused_);
    if (it == focusStack_.end() || it == focusStack_.begin()) {
        wm_.focusClient(focusStack_.back());
    } else {
        wm_.focusClient(*std::prev(it));
    }
}

void KeyFocus::pushFocusStack(ClientWindow* client) {
    // Remove if already present, then push to front
    removeFocusStack(client);
    focusStack_.insert(focusStack_.begin(), client);
}

void KeyFocus::removeFocusStack(ClientWindow* client) {
    auto it = std::find(focusStack_.begin(), focusStack_.end(), client);
    if (it != focusStack_.end()) {
        focusStack_.erase(it);
    }

    if (focused_ == client) {
        focused_ = nullptr;
    }
}

} // namespace motif::wm
