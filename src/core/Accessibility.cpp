#include <motif/core/Accessibility.h>
#include <motif/core/Widget.h>

#include <algorithm>

#ifdef HAVE_ATSPI
#include <atspi/atspi.h>
#include <dbus/dbus.h>
#endif

namespace motif::core {

AccessibilityBridge& AccessibilityBridge::instance() {
    static AccessibilityBridge inst;
    return inst;
}

bool AccessibilityBridge::initialize() {
#ifdef HAVE_ATSPI
    int status = atspi_init();
    if (status != 0) return false;
    active_ = true;
    return true;
#else
    // AT-SPI not available — accessibility bridge runs in stub mode.
    // All register/notify calls are accepted but produce no D-Bus traffic.
    active_ = false;
    return false;
#endif
}

void AccessibilityBridge::shutdown() {
#ifdef HAVE_ATSPI
    if (active_) {
        atspi_exit();
    }
#endif
    entries_.clear();
    active_ = false;
}

void AccessibilityBridge::registerWidget(Widget* widget, const AccessibleInfo& info) {
    if (!widget) return;

    // Check if already registered
    auto* existing = findEntry(widget);
    if (existing) {
        existing->info = info;
        return;
    }

    entries_.push_back({widget, info});

#ifdef HAVE_ATSPI
    if (active_) {
        // AT-SPI registration would go here via atspi_accessible APIs
        // For now, the bridge tracks state for future D-Bus emission
    }
#endif
}

void AccessibilityBridge::unregisterWidget(Widget* widget) {
    auto it = std::remove_if(entries_.begin(), entries_.end(),
        [widget](const WidgetEntry& e) { return e.widget == widget; });
    entries_.erase(it, entries_.end());
}

void AccessibilityBridge::notifyPropertyChanged(Widget* widget,
                                                  const std::string& property) {
    (void)property;
    auto* entry = findEntry(widget);
    if (!entry) return;

#ifdef HAVE_ATSPI
    if (active_) {
        // Emit PropertyChange event over D-Bus
    }
#endif
}

void AccessibilityBridge::notifyStateChanged(Widget* widget,
                                               AccessibleState state, bool set) {
    auto* entry = findEntry(widget);
    if (!entry) return;

    if (set) {
        entry->info.states = entry->info.states | state;
    } else {
        entry->info.states = static_cast<AccessibleState>(
            static_cast<uint32_t>(entry->info.states) &
            ~static_cast<uint32_t>(state));
    }

#ifdef HAVE_ATSPI
    if (active_) {
        // Emit StateChanged event
    }
#endif
}

void AccessibilityBridge::notifyValueChanged(Widget* widget,
                                               const std::string& newValue) {
    auto* entry = findEntry(widget);
    if (!entry) return;
    entry->info.value = newValue;

#ifdef HAVE_ATSPI
    if (active_) {
        // Emit object:property-change:accessible-value
    }
#endif
}

void AccessibilityBridge::notifyFocusChanged(Widget* widget, bool gained) {
    notifyStateChanged(widget, AccessibleState::Focused, gained);

#ifdef HAVE_ATSPI
    if (active_) {
        // Emit focus:gained or focus:lost
    }
#endif
}

void AccessibilityBridge::notifyTextChanged(Widget* widget, int offset,
                                              int length, const std::string& text) {
    (void)offset; (void)length; (void)text;
    auto* entry = findEntry(widget);
    if (!entry) return;

#ifdef HAVE_ATSPI
    if (active_) {
        // Emit object:text-changed:insert or object:text-changed:delete
    }
#endif
}

AccessibleInfo* AccessibilityBridge::getInfo(Widget* widget) {
    auto* entry = findEntry(widget);
    return entry ? &entry->info : nullptr;
}

void AccessibilityBridge::setAccessibleName(Widget* widget, const std::string& name) {
    auto* entry = findEntry(widget);
    if (entry) {
        entry->info.name = name;
        notifyPropertyChanged(widget, "accessible-name");
    }
}

void AccessibilityBridge::setAccessibleDescription(Widget* widget,
                                                     const std::string& desc) {
    auto* entry = findEntry(widget);
    if (entry) {
        entry->info.description = desc;
        notifyPropertyChanged(widget, "accessible-description");
    }
}

AccessibilityBridge::WidgetEntry* AccessibilityBridge::findEntry(Widget* widget) {
    for (auto& entry : entries_) {
        if (entry.widget == widget) return &entry;
    }
    return nullptr;
}

} // namespace motif::core
