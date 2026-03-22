#include <motif/core/Theme.h>
#include <motif/core/Log.h>

#include <X11/Xlib.h>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace motif::core {

// ── Theme ───────────────────────────────────────────────────

Theme::Theme(const std::string& name) : name_(name) {}

void Theme::setBaseStyle(const WidgetStyle& style) {
    baseStyle_ = style;
}

void Theme::setClassStyle(const std::string& className, const WidgetStyle& style) {
    classStyles_[className] = style;
}

const WidgetStyle* Theme::classStyle(const std::string& className) const {
    auto it = classStyles_.find(className);
    return (it != classStyles_.end()) ? &it->second : nullptr;
}

void Theme::setInstanceStyle(const std::string& instanceName, const WidgetStyle& style) {
    instanceStyles_[instanceName] = style;
}

const WidgetStyle* Theme::instanceStyle(const std::string& instanceName) const {
    auto it = instanceStyles_.find(instanceName);
    return (it != instanceStyles_.end()) ? &it->second : nullptr;
}

WidgetStyle Theme::resolveStyle(const std::string& className,
                                const std::string& instanceName) const {
    WidgetStyle result = baseStyle_;

    // Class override
    auto* cs = classStyle(className);
    if (cs) result = mergeStyles(result, *cs);

    // Instance override (highest priority)
    if (!instanceName.empty()) {
        auto* is = instanceStyle(instanceName);
        if (is) result = mergeStyles(result, *is);
    }

    return result;
}

WidgetStyle Theme::mergeStyles(const WidgetStyle& base, const WidgetStyle& overlay) {
    WidgetStyle result = base;

    if (!overlay.foreground.name.empty()) result.foreground = overlay.foreground;
    if (!overlay.background.name.empty()) result.background = overlay.background;
    if (!overlay.topShadow.name.empty()) result.topShadow = overlay.topShadow;
    if (!overlay.bottomShadow.name.empty()) result.bottomShadow = overlay.bottomShadow;
    if (!overlay.selectColor.name.empty()) result.selectColor = overlay.selectColor;
    if (!overlay.highlightColor.name.empty()) result.highlightColor = overlay.highlightColor;

    if (overlay.shadowThickness >= 0) result.shadowThickness = overlay.shadowThickness;
    if (overlay.highlightThickness >= 0) result.highlightThickness = overlay.highlightThickness;
    if (overlay.borderWidth >= 0) result.borderWidth = overlay.borderWidth;

    if (!overlay.fontName.empty()) result.fontName = overlay.fontName;

    if (overlay.marginWidth > 0) result.marginWidth = overlay.marginWidth;
    if (overlay.marginHeight > 0) result.marginHeight = overlay.marginHeight;
    if (overlay.spacing > 0) result.spacing = overlay.spacing;

    return result;
}

void Theme::realize(void* display, int screen) {
    auto* dpy = static_cast<Display*>(display);
    if (!dpy) return;

    auto resolveAll = [&](WidgetStyle& style) {
        resolveColor(display, screen, style.foreground);
        resolveColor(display, screen, style.background);
        resolveColor(display, screen, style.topShadow);
        resolveColor(display, screen, style.bottomShadow);
        resolveColor(display, screen, style.selectColor);
        resolveColor(display, screen, style.highlightColor);
    };

    resolveAll(baseStyle_);
    for (auto& [name, style] : classStyles_) resolveAll(style);
    for (auto& [name, style] : instanceStyles_) resolveAll(style);

    realized_ = true;
}

void Theme::resolveColor(void* display, int screen, ColorSpec& color) {
    if (color.resolved || color.name.empty()) return;

    auto* dpy = static_cast<Display*>(display);
    Colormap cmap = DefaultColormap(dpy, screen);
    XColor xc, exact;

    if (color.name[0] == '#' && color.name.size() == 7) {
        // Parse hex color
        unsigned int r, g, b;
        if (sscanf(color.name.c_str(), "#%02x%02x%02x", &r, &g, &b) == 3) {
            xc.red = static_cast<unsigned short>(r * 257);
            xc.green = static_cast<unsigned short>(g * 257);
            xc.blue = static_cast<unsigned short>(b * 257);
            xc.flags = DoRed | DoGreen | DoBlue;
            if (XAllocColor(dpy, cmap, &xc)) {
                color.pixel = xc.pixel;
                color.resolved = true;
                return;
            }
        }
    }

    // Try as named color
    if (XAllocNamedColor(dpy, cmap, color.name.c_str(), &xc, &exact)) {
        color.pixel = xc.pixel;
        color.resolved = true;
    } else {
        MOTIF_LOG_WARNING << "Cannot resolve color: " << color.name;
    }
}

ColorSpec Theme::parseColor(const std::string& value) {
    ColorSpec c;
    c.name = value;
    return c;
}

bool Theme::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        MOTIF_LOG_WARNING << "Cannot open theme file: " << path;
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    return loadFromString(content);
}

bool Theme::loadFromString(const std::string& content) {
    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line)) {
        // Skip comments and blank lines
        auto start = line.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        if (line[start] == '!' || line[start] == '#') continue;
        parseResourceLine(line);
    }
    return true;
}

void Theme::parseResourceLine(const std::string& line) {
    // Format: [*][ClassName.]resource: value
    auto colonPos = line.find(':');
    if (colonPos == std::string::npos) return;

    std::string lhs = line.substr(0, colonPos);
    std::string value = line.substr(colonPos + 1);

    // Trim whitespace
    auto trimStart = lhs.find_first_not_of(" \t*");
    if (trimStart == std::string::npos) return;
    lhs = lhs.substr(trimStart);

    auto valStart = value.find_first_not_of(" \t");
    if (valStart == std::string::npos) return;
    value = value.substr(valStart);
    auto valEnd = value.find_last_not_of(" \t\r\n");
    if (valEnd != std::string::npos) value = value.substr(0, valEnd + 1);

    // Split lhs into target.resource
    auto dotPos = lhs.find('.');
    std::string target;
    std::string resource;
    if (dotPos != std::string::npos) {
        target = lhs.substr(0, dotPos);
        resource = lhs.substr(dotPos + 1);
    } else {
        resource = lhs;
    }

    applyResource(target, resource, value);
}

void Theme::applyResource(const std::string& target,
                           const std::string& resource,
                           const std::string& value) {
    WidgetStyle* style = nullptr;

    if (target.empty()) {
        style = &baseStyle_;
    } else {
        // Check if it looks like a class name (starts with Xm or uppercase)
        if (target.substr(0, 2) == "Xm" || std::isupper(target[0])) {
            style = &classStyles_[target];
            style->name = target;
        } else {
            style = &instanceStyles_[target];
            style->name = target;
        }
    }

    if (!style) return;

    // Map resource names to style fields
    if (resource == "foreground" || resource == "Foreground") {
        style->foreground = parseColor(value);
    } else if (resource == "background" || resource == "Background") {
        style->background = parseColor(value);
    } else if (resource == "topShadowColor" || resource == "TopShadowColor") {
        style->topShadow = parseColor(value);
    } else if (resource == "bottomShadowColor" || resource == "BottomShadowColor") {
        style->bottomShadow = parseColor(value);
    } else if (resource == "selectColor" || resource == "SelectColor") {
        style->selectColor = parseColor(value);
    } else if (resource == "highlightColor" || resource == "HighlightColor") {
        style->highlightColor = parseColor(value);
    } else if (resource == "shadowThickness" || resource == "ShadowThickness") {
        style->shadowThickness = std::stoi(value);
    } else if (resource == "highlightThickness" || resource == "HighlightThickness") {
        style->highlightThickness = std::stoi(value);
    } else if (resource == "borderWidth" || resource == "BorderWidth") {
        style->borderWidth = std::stoi(value);
    } else if (resource == "fontList" || resource == "FontList" ||
               resource == "font" || resource == "Font") {
        style->fontName = value;
    } else if (resource == "marginWidth" || resource == "MarginWidth") {
        style->marginWidth = std::stoi(value);
    } else if (resource == "marginHeight" || resource == "MarginHeight") {
        style->marginHeight = std::stoi(value);
    } else if (resource == "spacing" || resource == "Spacing") {
        style->spacing = std::stoi(value);
    }
}

// ── Built-in themes ────────────────────────────────────────

Theme Theme::motifClassic() {
    Theme theme("MotifClassic");
    WidgetStyle base;
    base.background = {"#C4C4C4"};
    base.foreground = {"black"};
    base.topShadow = {"#E8E8E8"};
    base.bottomShadow = {"#808080"};
    base.selectColor = {"#B0B0B0"};
    base.highlightColor = {"black"};
    base.shadowThickness = 2;
    base.highlightThickness = 1;
    base.fontName = "fixed";
    theme.setBaseStyle(base);

    // Push button slightly lighter
    WidgetStyle btn;
    btn.background = {"#CCCCCC"};
    theme.setClassStyle("XmPushButton", btn);

    return theme;
}

Theme Theme::motifModern() {
    Theme theme("MotifModern");
    WidgetStyle base;
    base.background = {"#E0E0E0"};
    base.foreground = {"#1A1A1A"};
    base.topShadow = {"#F5F5F5"};
    base.bottomShadow = {"#999999"};
    base.selectColor = {"#4682B4"};
    base.highlightColor = {"#4682B4"};
    base.shadowThickness = 1;
    base.highlightThickness = 2;
    base.fontName = "Sans-11";
    theme.setBaseStyle(base);

    WidgetStyle btn;
    btn.background = {"#D0D0D0"};
    btn.shadowThickness = 2;
    theme.setClassStyle("XmPushButton", btn);

    WidgetStyle text;
    text.background = {"white"};
    text.foreground = {"black"};
    theme.setClassStyle("XmText", text);
    theme.setClassStyle("XmTextField", text);

    WidgetStyle menuBar;
    menuBar.background = {"#E8E8E8"};
    theme.setClassStyle("XmMenuBar", menuBar);

    return theme;
}

Theme Theme::motifDark() {
    Theme theme("MotifDark");
    WidgetStyle base;
    base.background = {"#2D2D2D"};
    base.foreground = {"#E0E0E0"};
    base.topShadow = {"#404040"};
    base.bottomShadow = {"#1A1A1A"};
    base.selectColor = {"#4A90D9"};
    base.highlightColor = {"#4A90D9"};
    base.shadowThickness = 1;
    base.highlightThickness = 2;
    base.fontName = "Sans-11";
    theme.setBaseStyle(base);

    WidgetStyle btn;
    btn.background = {"#383838"};
    theme.setClassStyle("XmPushButton", btn);

    WidgetStyle text;
    text.background = {"#1E1E1E"};
    text.foreground = {"#D4D4D4"};
    theme.setClassStyle("XmText", text);
    theme.setClassStyle("XmTextField", text);

    WidgetStyle list;
    list.background = {"#252525"};
    list.foreground = {"#CCCCCC"};
    theme.setClassStyle("XmList", list);

    return theme;
}

Theme Theme::motifHighContrast() {
    Theme theme("MotifHighContrast");
    WidgetStyle base;
    base.background = {"white"};
    base.foreground = {"black"};
    base.topShadow = {"black"};
    base.bottomShadow = {"black"};
    base.selectColor = {"#FFFF00"};
    base.highlightColor = {"#0000FF"};
    base.shadowThickness = 2;
    base.highlightThickness = 3;
    base.borderWidth = 1;
    base.fontName = "Sans-14:bold";
    theme.setBaseStyle(base);

    WidgetStyle btn;
    btn.borderWidth = 2;
    theme.setClassStyle("XmPushButton", btn);

    return theme;
}

// ── ThemeManager ────────────────────────────────────────────

ThemeManager::ThemeManager() {
    // Register built-in themes
    registerTheme("MotifClassic", Theme::motifClassic());
    registerTheme("MotifModern", Theme::motifModern());
    registerTheme("MotifDark", Theme::motifDark());
    registerTheme("MotifHighContrast", Theme::motifHighContrast());

    // Default to modern
    activeTheme_ = Theme::motifModern();
}

ThemeManager& ThemeManager::instance() {
    static ThemeManager mgr;
    return mgr;
}

void ThemeManager::registerTheme(const std::string& name, Theme theme) {
    themes_[name] = std::move(theme);
}

bool ThemeManager::setActiveTheme(const std::string& name) {
    auto it = themes_.find(name);
    if (it == themes_.end()) return false;
    activeTheme_ = it->second;
    notifyThemeChanged();
    return true;
}

void ThemeManager::setActiveTheme(Theme theme) {
    activeTheme_ = std::move(theme);
    notifyThemeChanged();
}

const Theme* ThemeManager::findTheme(const std::string& name) const {
    auto it = themes_.find(name);
    return (it != themes_.end()) ? &it->second : nullptr;
}

std::vector<std::string> ThemeManager::themeNames() const {
    std::vector<std::string> names;
    names.reserve(themes_.size());
    for (const auto& [name, _] : themes_) {
        names.push_back(name);
    }
    return names;
}

void ThemeManager::onThemeChanged(ThemeChangedCallback callback) {
    callbacks_.push_back(std::move(callback));
}

void ThemeManager::notifyThemeChanged() {
    for (auto& cb : callbacks_) {
        cb(activeTheme_);
    }
}

} // namespace motif::core
