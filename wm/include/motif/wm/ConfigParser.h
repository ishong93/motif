#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace motif::wm {

/// Parsed .mwmrc configuration.
/// Supports Motif-style menu definitions, key bindings, and button bindings.
struct MwmConfig {
    /// Menu item: label + function name + optional argument
    struct MenuItem {
        std::string label;
        std::string mnemonic;      // single character
        std::string accelerator;   // e.g., "Alt<Key>F4"
        std::string function;      // e.g., "f.close", "f.exec"
        std::string argument;      // e.g., "xterm"
        bool isSeparator = false;
    };

    /// Named menu definition
    struct MenuDef {
        std::string name;
        std::vector<MenuItem> items;
    };

    /// Key binding: modifier + key → function
    struct KeyBinding {
        unsigned int modifiers = 0;  // ShiftMask, ControlMask, Mod1Mask, etc.
        std::string keysym;          // e.g., "F4", "Tab"
        std::string context;         // "root", "window", "icon", "frame"
        std::string function;
        std::string argument;
    };

    /// Button binding: modifier + button → function
    struct ButtonBinding {
        unsigned int modifiers = 0;
        int button = 0;             // 1, 2, 3
        std::string context;
        std::string function;
        std::string argument;
    };

    std::unordered_map<std::string, MenuDef> menus;
    std::vector<KeyBinding> keys;
    std::vector<ButtonBinding> buttons;

    /// Resource settings (e.g., Mwm*keyboardFocusPolicy)
    std::unordered_map<std::string, std::string> resources;
};

/// Parser for .mwmrc configuration files.
class ConfigParser {
public:
    /// Parse a .mwmrc file
    bool parseFile(const std::string& path);

    /// Parse from string
    bool parseString(const std::string& content);

    const MwmConfig& config() const { return config_; }
    const std::vector<std::string>& errors() const { return errors_; }

    /// Find default config file (~/.mwmrc or system default)
    static std::string findConfigFile();

private:
    void parseLine(const std::string& line, int lineNum);
    void parseMenuDef(const std::string& name, int startLine);
    void parseMenuItem(const std::string& line, int lineNum);
    void parseKeyBinding(const std::string& line, int lineNum);
    void parseButtonBinding(const std::string& line, int lineNum);
    void parseResource(const std::string& line, int lineNum);

    unsigned int parseModifiers(const std::string& modStr) const;

    MwmConfig config_;
    std::vector<std::string> errors_;
    std::vector<std::string> lines_;
    int currentLine_ = 0;
    std::string currentMenu_;

    enum class Section { None, Menu, Keys, Buttons };
    Section section_ = Section::None;
};

} // namespace motif::wm
