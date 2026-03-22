#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>

namespace motif::core {

/// Color specification — can be named color, hex (#RRGGBB), or X11 pixel value
struct ColorSpec {
    std::string name;           // "SteelBlue", "#4682B4", etc.
    unsigned long pixel = 0;    // resolved X11 pixel value
    bool resolved = false;
};

/// Widget style definition — visual properties for a widget class or instance
struct WidgetStyle {
    std::string name;

    // Colors
    ColorSpec foreground;
    ColorSpec background;
    ColorSpec topShadow;
    ColorSpec bottomShadow;
    ColorSpec selectColor;
    ColorSpec highlightColor;

    // Border
    int shadowThickness = 2;
    int highlightThickness = 1;
    int borderWidth = 0;

    // Font
    std::string fontName;

    // Spacing
    int marginWidth = 0;
    int marginHeight = 0;
    int spacing = 0;
};

/// Theme — a complete visual configuration consisting of a base style
/// and per-widget-class style overrides. Supports loading from Motif-style
/// resource files (e.g., *background: #E0E0E0).
class Theme {
public:
    Theme() = default;
    explicit Theme(const std::string& name);

    /// Theme name
    const std::string& name() const { return name_; }

    /// Set base (default) style
    void setBaseStyle(const WidgetStyle& style);
    const WidgetStyle& baseStyle() const { return baseStyle_; }

    /// Set style override for a specific widget class (e.g., "XmPushButton")
    void setClassStyle(const std::string& className, const WidgetStyle& style);
    const WidgetStyle* classStyle(const std::string& className) const;

    /// Set style override for a specific named widget instance
    void setInstanceStyle(const std::string& instanceName, const WidgetStyle& style);
    const WidgetStyle* instanceStyle(const std::string& instanceName) const;

    /// Resolve a style for a widget: instance > class > base
    WidgetStyle resolveStyle(const std::string& className,
                             const std::string& instanceName = {}) const;

    /// Resolve all colors against a Display/Colormap
    void realize(void* display, int screen);

    /// Load theme from a Motif-style resource file
    /// Format: resource lines like:
    ///   *background: #E0E0E0
    ///   *XmPushButton.background: #C0C0C0
    ///   *fontList: Sans-11
    bool loadFromFile(const std::string& path);

    /// Load from a string buffer (same format)
    bool loadFromString(const std::string& content);

    // ── Built-in themes ──────────────────────────────────────
    static Theme motifClassic();    // Classic Motif gray
    static Theme motifModern();     // Updated blue/gray
    static Theme motifDark();       // Dark theme
    static Theme motifHighContrast(); // Accessibility theme

private:
    void parseResourceLine(const std::string& line);
    void applyResource(const std::string& target,
                       const std::string& resource,
                       const std::string& value);
    static ColorSpec parseColor(const std::string& value);
    static void resolveColor(void* display, int screen, ColorSpec& color);

    // Merge non-default fields of overlay onto base
    static WidgetStyle mergeStyles(const WidgetStyle& base,
                                   const WidgetStyle& overlay);

    std::string name_;
    WidgetStyle baseStyle_;
    std::unordered_map<std::string, WidgetStyle> classStyles_;
    std::unordered_map<std::string, WidgetStyle> instanceStyles_;
    bool realized_ = false;
};

/// ThemeManager — singleton managing the active theme and theme registry
class ThemeManager {
public:
    static ThemeManager& instance();

    /// Register a named theme
    void registerTheme(const std::string& name, Theme theme);

    /// Set the active theme by name
    bool setActiveTheme(const std::string& name);

    /// Set active theme directly
    void setActiveTheme(Theme theme);

    /// Get the active theme
    const Theme& activeTheme() const { return activeTheme_; }
    Theme& activeTheme() { return activeTheme_; }

    /// Get a registered theme by name
    const Theme* findTheme(const std::string& name) const;

    /// List registered theme names
    std::vector<std::string> themeNames() const;

    /// Register a callback for theme changes
    using ThemeChangedCallback = std::function<void(const Theme&)>;
    void onThemeChanged(ThemeChangedCallback callback);

private:
    ThemeManager();
    ~ThemeManager() = default;
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void notifyThemeChanged();

    Theme activeTheme_;
    std::unordered_map<std::string, Theme> themes_;
    std::vector<ThemeChangedCallback> callbacks_;
};

} // namespace motif::core
