#include <motif/input/KeyBinding.h>
#include <fstream>
#include <sstream>

namespace motif {

bool KeyBindingTable::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '!') continue;

        // Parse "osfKeyName : [modifiers] <Key>X11KeyName"
        auto colonPos = line.find(':');
        if (colonPos == std::string::npos) continue;

        VirtualKeyMapping mapping;

        // Extract osfKeyName (trim whitespace)
        mapping.osfKeyName = line.substr(0, colonPos);
        auto start = mapping.osfKeyName.find_first_not_of(" \t");
        auto end = mapping.osfKeyName.find_last_not_of(" \t");
        if (start != std::string::npos)
            mapping.osfKeyName = mapping.osfKeyName.substr(start, end - start + 1);

        // Parse right side for modifiers and key
        std::string rhs = line.substr(colonPos + 1);
        auto keyTagPos = rhs.find("<Key>");
        if (keyTagPos == std::string::npos) continue;

        // Modifiers are everything before <Key>
        std::string mods = rhs.substr(0, keyTagPos);
        start = mods.find_first_not_of(" \t");
        end = mods.find_last_not_of(" \t");
        if (start != std::string::npos)
            mapping.modifiers = mods.substr(start, end - start + 1);

        // Key name is everything after <Key>
        std::string keyName = rhs.substr(keyTagPos + 5);
        start = keyName.find_first_not_of(" \t");
        end = keyName.find_last_not_of(" \t\r\n");
        if (start != std::string::npos)
            mapping.x11KeyName = keyName.substr(start, end - start + 1);

        addMapping(mapping);
    }

    return true;
}

void KeyBindingTable::addMapping(const VirtualKeyMapping& mapping) {
    osfKeyIndex_[mapping.osfKeyName] = mappings_.size();
    mappings_.push_back(mapping);
}

const VirtualKeyMapping* KeyBindingTable::findByOsfKey(const std::string& osfKey) const {
    auto it = osfKeyIndex_.find(osfKey);
    if (it != osfKeyIndex_.end()) {
        return &mappings_[it->second];
    }
    return nullptr;
}

} // namespace motif
