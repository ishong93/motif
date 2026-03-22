#include <motif/wm/ConfigParser.h>

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

#include <X11/Xlib.h>

namespace motif::wm {

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return {};
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::vector<std::string> splitTokens(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;

    while (iss >> token) {
        // Handle quoted strings
        if (token.front() == '"') {
            if (token.back() == '"' && token.size() > 1) {
                tokens.push_back(token.substr(1, token.size() - 2));
            } else {
                std::string quoted = token.substr(1);
                std::string rest;
                while (std::getline(iss, rest, '"')) {
                    quoted += " " + rest;
                    break;
                }
                tokens.push_back(quoted);
            }
        } else {
            tokens.push_back(token);
        }
    }
    return tokens;
}

std::string ConfigParser::findConfigFile() {
    // Try ~/.mwmrc first
    const char* home = std::getenv("HOME");
    if (home) {
        std::string path = std::string(home) + "/.mwmrc";
        std::ifstream test(path);
        if (test.good()) return path;
    }

    // Try system default
    std::string sysDefault = "/usr/share/motif/system.mwmrc";
    std::ifstream test(sysDefault);
    if (test.good()) return sysDefault;

    return {};
}

bool ConfigParser::parseFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        errors_.push_back("cannot open config file: " + path);
        return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    return parseString(ss.str());
}

bool ConfigParser::parseString(const std::string& content) {
    lines_.clear();
    errors_.clear();
    config_ = {};
    section_ = Section::None;
    currentMenu_.clear();

    // Split into lines, handling line continuation (\)
    std::istringstream stream(content);
    std::string line;
    std::string accumulated;

    while (std::getline(stream, line)) {
        // Remove trailing \r
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Handle line continuation
        if (!line.empty() && line.back() == '\\') {
            accumulated += line.substr(0, line.size() - 1) + " ";
            continue;
        }

        accumulated += line;
        lines_.push_back(accumulated);
        accumulated.clear();
    }
    if (!accumulated.empty()) {
        lines_.push_back(accumulated);
    }

    // Parse each line
    for (int i = 0; i < static_cast<int>(lines_.size()); ++i) {
        currentLine_ = i;
        parseLine(lines_[i], i + 1);
    }

    return errors_.empty();
}

void ConfigParser::parseLine(const std::string& rawLine, int lineNum) {
    std::string line = trim(rawLine);

    // Skip empty lines and comments
    if (line.empty() || line[0] == '!') return;

    // Check for section headers
    if (line.front() == '<' || line.find("Menu ") == 0 || line.find("menu ") == 0) {
        // Menu definition: "Menu MenuName" or "Menu MenuName {"
        if (line.find("enu ") != std::string::npos) {
            auto tokens = splitTokens(line);
            if (tokens.size() >= 2) {
                currentMenu_ = tokens[1];
                config_.menus[currentMenu_] = {currentMenu_, {}};
                section_ = Section::Menu;
                return;
            }
        }
    }

    // Check for "Keys" section
    if (line == "Keys DefaultKeyBindings" || line == "Keys" ||
        line.find("Keys ") == 0) {
        section_ = Section::Keys;
        return;
    }

    // Check for "Buttons" section
    if (line == "Buttons DefaultButtonBindings" || line == "Buttons" ||
        line.find("Buttons ") == 0) {
        section_ = Section::Buttons;
        return;
    }

    // Opening/closing braces
    if (line == "{") return;
    if (line == "}") {
        section_ = Section::None;
        currentMenu_.clear();
        return;
    }

    // Resource setting (Mwm*resource: value)
    if (line.find("Mwm*") == 0 || line.find("mwm*") == 0) {
        parseResource(line, lineNum);
        return;
    }

    // Dispatch based on current section
    switch (section_) {
        case Section::Menu:
            parseMenuItem(line, lineNum);
            break;
        case Section::Keys:
            parseKeyBinding(line, lineNum);
            break;
        case Section::Buttons:
            parseButtonBinding(line, lineNum);
            break;
        case Section::None:
            // Could be a standalone resource
            if (line.find(':') != std::string::npos) {
                parseResource(line, lineNum);
            }
            break;
    }
}

void ConfigParser::parseMenuItem(const std::string& line, int /*lineNum*/) {
    if (currentMenu_.empty()) return;

    // Check for separator
    std::string lower = line;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower.find("no-label") != std::string::npos ||
        lower.find("separator") != std::string::npos ||
        lower == "---") {
        MwmConfig::MenuItem item;
        item.isSeparator = true;
        config_.menus[currentMenu_].items.push_back(item);
        return;
    }

    // Format: "label" [_mnemonic] [accelerator] function [argument]
    auto tokens = splitTokens(line);
    if (tokens.empty()) return;

    MwmConfig::MenuItem item;
    item.label = tokens[0];

    size_t idx = 1;

    // Check for mnemonic (_X)
    if (idx < tokens.size() && tokens[idx].size() == 2 && tokens[idx][0] == '_') {
        item.mnemonic = std::string(1, tokens[idx][1]);
        ++idx;
    }

    // Check for accelerator (contains <Key>)
    if (idx < tokens.size() && tokens[idx].find("<Key>") != std::string::npos) {
        item.accelerator = tokens[idx];
        ++idx;
    }

    // Function
    if (idx < tokens.size()) {
        item.function = tokens[idx];
        ++idx;
    }

    // Argument (rest of line)
    if (idx < tokens.size()) {
        item.argument = tokens[idx];
        for (size_t i = idx + 1; i < tokens.size(); ++i) {
            item.argument += " " + tokens[i];
        }
    }

    config_.menus[currentMenu_].items.push_back(item);
}

void ConfigParser::parseKeyBinding(const std::string& line, int /*lineNum*/) {
    // Format: modifier<Key>keysym context function [argument]
    auto tokens = splitTokens(line);
    if (tokens.size() < 3) return;

    MwmConfig::KeyBinding binding;

    // Parse modifier+key (e.g., "Alt<Key>F4")
    std::string keySpec = tokens[0];
    size_t keyPos = keySpec.find("<Key>");
    if (keyPos == std::string::npos) return;

    std::string modStr = keySpec.substr(0, keyPos);
    binding.modifiers = parseModifiers(modStr);
    binding.keysym = keySpec.substr(keyPos + 5);

    binding.context = tokens[1];
    binding.function = tokens[2];
    if (tokens.size() > 3) {
        binding.argument = tokens[3];
    }

    config_.keys.push_back(binding);
}

void ConfigParser::parseButtonBinding(const std::string& line, int /*lineNum*/) {
    // Format: modifier<Btn1Down> context function [argument]
    auto tokens = splitTokens(line);
    if (tokens.size() < 3) return;

    MwmConfig::ButtonBinding binding;

    std::string btnSpec = tokens[0];
    size_t btnPos = btnSpec.find("<Btn");
    if (btnPos == std::string::npos) return;

    std::string modStr = btnSpec.substr(0, btnPos);
    binding.modifiers = parseModifiers(modStr);

    // Extract button number
    size_t numPos = btnPos + 4;
    if (numPos < btnSpec.size() && btnSpec[numPos] >= '1' && btnSpec[numPos] <= '5') {
        binding.button = btnSpec[numPos] - '0';
    }

    binding.context = tokens[1];
    binding.function = tokens[2];
    if (tokens.size() > 3) {
        binding.argument = tokens[3];
    }

    config_.buttons.push_back(binding);
}

void ConfigParser::parseResource(const std::string& line, int /*lineNum*/) {
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) return;

    std::string key = trim(line.substr(0, colonPos));
    std::string value = trim(line.substr(colonPos + 1));

    config_.resources[key] = value;
}

unsigned int ConfigParser::parseModifiers(const std::string& modStr) const {
    unsigned int mods = 0;

    std::string lower = modStr;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower.find("shift") != std::string::npos) mods |= ShiftMask;
    if (lower.find("ctrl") != std::string::npos ||
        lower.find("control") != std::string::npos) mods |= ControlMask;
    if (lower.find("alt") != std::string::npos ||
        lower.find("meta") != std::string::npos ||
        lower.find("mod1") != std::string::npos) mods |= Mod1Mask;
    if (lower.find("mod2") != std::string::npos) mods |= Mod2Mask;
    if (lower.find("mod3") != std::string::npos) mods |= Mod3Mask;
    if (lower.find("mod4") != std::string::npos ||
        lower.find("super") != std::string::npos) mods |= Mod4Mask;
    if (lower.find("mod5") != std::string::npos) mods |= Mod5Mask;

    return mods;
}

} // namespace motif::wm
