#pragma once

#include <any>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <variant>
#include <vector>

namespace motif {

// ── ResourceValue: a tagged union for common Motif resource types ────
// Replaces Xt's XrmValue + type-string with a type-safe variant.

using ResourceValue = std::variant<
    bool,
    int,
    unsigned int,
    float,
    double,
    std::string,
    unsigned long,        // Pixel, XID
    std::vector<uint8_t>  // raw binary data
>;

// ── ResourceConverter: replaces Xt CvtStringToXXX converters ─────────
// Legacy Motif used XtSetTypeConverter for string→enum, string→pixel, etc.
// This system uses registered lambdas keyed by (fromType, toType).

class ResourceConverter {
public:
    static ResourceConverter& instance();

    using ConvertFunc = std::function<std::optional<ResourceValue>(const ResourceValue& from)>;

    struct TypePair {
        std::type_index from;
        std::type_index to;
        bool operator==(const TypePair& o) const { return from == o.from && to == o.to; }
    };

    struct TypePairHash {
        size_t operator()(const TypePair& p) const {
            return std::hash<std::type_index>{}(p.from) ^ (std::hash<std::type_index>{}(p.to) << 1);
        }
    };

    // Register a converter from type F to type T
    template <typename F, typename T>
    void registerConverter(std::function<std::optional<T>(const F&)> func) {
        TypePair key{typeid(F), typeid(T)};
        converters_[key] = [func](const ResourceValue& v) -> std::optional<ResourceValue> {
            if (auto* val = std::get_if<F>(&v)) {
                auto result = func(*val);
                if (result) return ResourceValue{*result};
            }
            return std::nullopt;
        };
    }

    // Try to convert a value
    std::optional<ResourceValue> convert(std::type_index fromType,
                                          std::type_index toType,
                                          const ResourceValue& value) const;

    // Register built-in converters (string→int, string→bool, string→pixel, etc.)
    void registerBuiltins();

private:
    ResourceConverter();
    std::unordered_map<TypePair, ConvertFunc, TypePairHash> converters_;
};

// ── ResourceSpec: describes one resource of a widget class ───────────
// Replaces XtResource struct from Xt Intrinsics.

struct ResourceSpec {
    std::string name;             // e.g. "shadowThickness"
    std::string className;        // e.g. "ShadowThickness"
    std::type_index type;         // target C++ type
    size_t offset;                // byte offset into widget struct (for reflection)
    ResourceValue defaultValue;   // default if not specified

    ResourceSpec(std::string n, std::string cn, std::type_index t,
                 size_t off, ResourceValue def)
        : name(std::move(n)), className(std::move(cn)),
          type(t), offset(off), defaultValue(std::move(def)) {}
};

// ── ResourceDatabase: hierarchical resource lookup ───────────────────
// Replaces Xrm database. Supports "widget.class.resource: value" patterns.

class ResourceDatabase {
public:
    static ResourceDatabase& instance();

    // Store a resource value: "app.form.button.background: red"
    void put(const std::string& qualifiedName, const ResourceValue& value);

    // Lookup: returns most specific match for widget path + resource name
    std::optional<ResourceValue> lookup(const std::string& widgetPath,
                                         const std::string& resourceName) const;

    // Load from X resource string format (app-defaults file)
    bool loadFromString(const std::string& data);
    bool loadFromFile(const std::string& path);

    // Merge with X server resources
    bool loadFromDisplay();

    void clear();

private:
    ResourceDatabase() = default;

    struct Entry {
        std::string pattern;   // e.g. "*background", "app.form*foreground"
        ResourceValue value;
        int specificity;       // higher = more specific
    };

    std::vector<Entry> entries_;
    int matchSpecificity(const std::string& pattern,
                          const std::string& widgetPath,
                          const std::string& resourceName) const;
};

// ── Resource: per-widget resource bag (enhanced from original) ───────

class Resource {
public:
    template <typename T>
    void set(const std::string& name, T value) {
        values_[name] = std::move(value);
    }

    template <typename T>
    T get(const std::string& name, const T& defaultValue = T{}) const {
        auto it = values_.find(name);
        if (it != values_.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    bool has(const std::string& name) const {
        return values_.find(name) != values_.end();
    }

    void remove(const std::string& name) {
        values_.erase(name);
    }

    // Apply resource specs from database for a given widget path
    void applyDefaults(const std::string& widgetPath,
                       const std::vector<ResourceSpec>& specs);

    // Bulk set from a map of string values (auto-converts via ResourceConverter)
    void setFromStrings(const std::unordered_map<std::string, std::string>& values,
                        const std::vector<ResourceSpec>& specs);

private:
    std::unordered_map<std::string, std::any> values_;
};

} // namespace motif
