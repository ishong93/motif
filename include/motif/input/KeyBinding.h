#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace motif {

struct VirtualKeyMapping {
    std::string osfKeyName;    // e.g. "osfLeft"
    std::string modifiers;     // e.g. "Alt"
    std::string x11KeyName;    // e.g. "Left"
};

class KeyBindingTable {
public:
    bool loadFromFile(const std::string& path);

    void addMapping(const VirtualKeyMapping& mapping);
    const VirtualKeyMapping* findByOsfKey(const std::string& osfKey) const;
    const std::vector<VirtualKeyMapping>& mappings() const { return mappings_; }

private:
    std::vector<VirtualKeyMapping> mappings_;
    std::unordered_map<std::string, size_t> osfKeyIndex_;
};

} // namespace motif
