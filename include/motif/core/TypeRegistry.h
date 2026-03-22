#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace motif {

// ── TypeRegistry ────────────────────────────────────────────────────
// Replaces legacy Motif RepType.c — a registry for enum-like
// "representation types" used by widget resources.
//
// Each registered type maps a set of string names to integer values
// (and back).  This is the backbone of resource conversion: when a
// user writes `*alignment: alignment_center` in a resource file,
// the TypeRegistry resolves "alignment_center" → 1.
//
// Key differences from legacy:
//   1. std::string throughout — no raw char*
//   2. Case-insensitive lookup by default
//   3. No Xt dependency — standalone registry
//   4. Thread-safe singleton access
//   5. Supports both consecutive (0,1,2,...) and mapped value sets

using RepTypeId = uint16_t;
static constexpr RepTypeId REP_TYPE_INVALID = 0x1FFF;

// ── RepTypeEntry: one registered representation type ────────────────

struct RepTypeEntry {
    std::string name;                          // e.g. "Alignment"
    std::vector<std::string> valueNames;       // e.g. {"alignment_beginning", "alignment_center", ...}
    std::vector<uint8_t> valueMap;             // maps index → actual numeric value (empty if consecutive from 0)
    RepTypeId id = REP_TYPE_INVALID;

    // Resolve a string name to its numeric value
    std::optional<uint8_t> nameToValue(const std::string& valueName) const;

    // Resolve a numeric value to its string name
    std::optional<std::string> valueToName(uint8_t value) const;

    // Number of registered value names
    size_t numValues() const { return valueNames.size(); }

    // Whether this type uses a non-consecutive value map
    bool isMapped() const { return !valueMap.empty(); }
};

// ── TypeRegistry: singleton registry ────────────────────────────────

class TypeRegistry {
public:
    static TypeRegistry& instance();

    // Register a new representation type (consecutive values 0..N-1)
    RepTypeId registerType(const std::string& typeName,
                           const std::vector<std::string>& valueNames);

    // Register a mapped representation type (arbitrary value assignments)
    RepTypeId registerMappedType(const std::string& typeName,
                                 const std::vector<std::string>& valueNames,
                                 const std::vector<uint8_t>& values);

    // Look up a type by name
    const RepTypeEntry* findType(const std::string& typeName) const;

    // Look up a type by ID
    const RepTypeEntry* findType(RepTypeId id) const;

    // Get the ID for a type name (REP_TYPE_INVALID if not found)
    RepTypeId getId(const std::string& typeName) const;

    // Convert a string value to numeric for a given type
    std::optional<uint8_t> stringToValue(const std::string& typeName,
                                          const std::string& valueName) const;
    std::optional<uint8_t> stringToValue(RepTypeId id,
                                          const std::string& valueName) const;

    // Convert a numeric value to string for a given type
    std::optional<std::string> valueToString(const std::string& typeName,
                                              uint8_t value) const;
    std::optional<std::string> valueToString(RepTypeId id,
                                              uint8_t value) const;

    // Get all registered type entries (for iteration/debugging)
    const std::vector<RepTypeEntry>& allTypes() const { return types_; }

    // Register all standard Motif representation types
    void registerBuiltins();

private:
    TypeRegistry() = default;

    std::vector<RepTypeEntry> types_;
    std::unordered_map<std::string, RepTypeId> nameToId_;  // lowercase name → ID
    bool builtinsRegistered_ = false;
};

} // namespace motif
