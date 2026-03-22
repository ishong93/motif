#pragma once

#include <X11/Xlib.h>
#include <string>
#include <functional>
#include <unordered_map>

namespace motif::core {

/// Pre-edit (composition) information for input method feedback
struct PreeditInfo {
    std::string text;           // current preedit string (UTF-8)
    int caretPos = 0;           // cursor position within preedit text
    bool active = false;        // preedit session is active
};

/// XIM input method wrapper for internationalized text input.
/// Manages the XIM connection, creates per-window XIC (Input Context),
/// and handles preedit/status callbacks for on-the-spot input.
class InputMethod {
public:
    static InputMethod& instance();

    /// Initialize XIM for the display. Call after XOpenDisplay.
    /// Returns false if no input method is available.
    bool initialize(Display* dpy);

    /// Shutdown and close XIM
    void shutdown();

    /// Create an input context for a window.
    /// The window should already be realized (mapped).
    XIC createInputContext(Window w);

    /// Destroy input context for a window
    void destroyInputContext(Window w);

    /// Get the input context for a window
    XIC getInputContext(Window w) const;

    /// Process a KeyPress event through XIM, returning the composed UTF-8 string.
    /// Returns the number of characters in the result string.
    /// keysym is set to the resulting KeySym (NoSymbol if consumed by IM).
    int lookupString(XIC ic, XKeyPressedEvent& event,
                     std::string& result, KeySym& keysym);

    /// Filter an event through XIM (call before normal event dispatch).
    /// Returns true if the event was consumed by the input method.
    bool filterEvent(XEvent& event);

    /// Get current preedit state for a window
    const PreeditInfo& preeditInfo(Window w) const;

    /// Register callback for preedit text changes (for widgets to render composition)
    using PreeditCallback = std::function<void(Window w, const PreeditInfo& info)>;
    void onPreeditChanged(PreeditCallback callback);

    /// Check if XIM is available
    bool isAvailable() const { return xim_ != nullptr; }

    /// Get the XIM style being used
    XIMStyle inputStyle() const { return inputStyle_; }

    /// Set the focus window for an IC
    void setFocus(XIC ic);
    void unsetFocus(XIC ic);

private:
    InputMethod() = default;
    ~InputMethod();
    InputMethod(const InputMethod&) = delete;
    InputMethod& operator=(const InputMethod&) = delete;

    XIMStyle chooseInputStyle() const;

    Display* dpy_ = nullptr;
    XIM xim_ = nullptr;
    XIMStyle inputStyle_ = 0;

    std::unordered_map<Window, XIC> contexts_;
    std::unordered_map<Window, PreeditInfo> preeditState_;
    std::vector<PreeditCallback> preeditCallbacks_;

    static PreeditInfo emptyPreedit_;
};

} // namespace motif::core
