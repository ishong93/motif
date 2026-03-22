#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace motif {
class Widget;
}

namespace motif::core {

/// Accessibility role for AT-SPI registration.
enum class AccessibleRole {
    Unknown,
    Window,
    Frame,
    Dialog,
    PushButton,
    ToggleButton,
    CheckBox,
    RadioButton,
    Label,
    TextField,
    TextArea,
    List,
    ListItem,
    ComboBox,
    MenuBar,
    Menu,
    MenuItem,
    Separator,
    ScrollBar,
    Scale,
    ProgressBar,
    Toolbar,
    StatusBar,
    TabList,
    TabPanel,
    Panel,
    Tree,
    TreeItem,
    Icon,
};

/// Accessibility state flags (bitfield).
enum class AccessibleState : uint32_t {
    None          = 0,
    Enabled       = 1 << 0,
    Visible       = 1 << 1,
    Showing       = 1 << 2,
    Focused       = 1 << 3,
    Focusable     = 1 << 4,
    Checked       = 1 << 5,
    Selected      = 1 << 6,
    Editable      = 1 << 7,
    Expandable    = 1 << 8,
    Expanded       = 1 << 9,
    Sensitive     = 1 << 10,
    MultiLine     = 1 << 11,
};

inline AccessibleState operator|(AccessibleState a, AccessibleState b) {
    return static_cast<AccessibleState>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline bool hasState(AccessibleState states, AccessibleState flag) {
    return (static_cast<uint32_t>(states) & static_cast<uint32_t>(flag)) != 0;
}

/// Per-widget accessibility information.
struct AccessibleInfo {
    AccessibleRole role = AccessibleRole::Unknown;
    AccessibleState states = AccessibleState::None;
    std::string name;           // accessible name (e.g., button label)
    std::string description;    // longer description
    std::string value;          // current value (for scales, text fields)
    std::string actionName;     // primary action (e.g., "press", "toggle")
};

/// Accessibility bridge — registers widgets with the platform
/// accessibility system (AT-SPI on Linux via D-Bus).
/// Compile-time optional via HAVE_ATSPI.
class AccessibilityBridge {
public:
    static AccessibilityBridge& instance();

    /// Initialize the accessibility bridge
    bool initialize();

    /// Shutdown and unregister
    void shutdown();

    bool isActive() const { return active_; }

    /// Register a widget with the accessibility system
    void registerWidget(Widget* widget, const AccessibleInfo& info);

    /// Unregister a widget
    void unregisterWidget(Widget* widget);

    /// Notify that a widget's accessible properties changed
    void notifyPropertyChanged(Widget* widget, const std::string& property);

    /// Notify that a widget's state changed
    void notifyStateChanged(Widget* widget, AccessibleState state, bool active);

    /// Notify that a widget's value changed
    void notifyValueChanged(Widget* widget, const std::string& newValue);

    /// Notify that focus changed
    void notifyFocusChanged(Widget* widget, bool gained);

    /// Notify that text changed in a text widget
    void notifyTextChanged(Widget* widget, int offset, int length,
                           const std::string& text);

    /// Get accessible info for a widget (nullptr if not registered)
    AccessibleInfo* getInfo(Widget* widget);

    /// Update the accessible name (e.g., when label changes)
    void setAccessibleName(Widget* widget, const std::string& name);

    /// Update the accessible description
    void setAccessibleDescription(Widget* widget, const std::string& desc);

private:
    AccessibilityBridge() = default;

    bool active_ = false;

    struct WidgetEntry {
        Widget* widget = nullptr;
        AccessibleInfo info;
    };

    std::vector<WidgetEntry> entries_;
    WidgetEntry* findEntry(Widget* widget);
};

} // namespace motif::core
