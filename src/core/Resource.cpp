#include <motif/core/Resource.h>
#include <motif/core/Application.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace motif {

// ── ResourceConverter ────────────────────────────────────────────────

ResourceConverter::ResourceConverter() {
    registerBuiltins();
}

ResourceConverter& ResourceConverter::instance() {
    static ResourceConverter inst;
    return inst;
}

std::optional<ResourceValue> ResourceConverter::convert(
    std::type_index fromType, std::type_index toType,
    const ResourceValue& value) const
{
    TypePair key{fromType, toType};
    auto it = converters_.find(key);
    if (it != converters_.end()) {
        return it->second(value);
    }
    return std::nullopt;
}

static std::string toLower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return r;
}

void ResourceConverter::registerBuiltins() {
    // string → bool
    registerConverter<std::string, bool>([](const std::string& s) -> std::optional<bool> {
        auto low = toLower(s);
        if (low == "true" || low == "on" || low == "yes" || low == "1") return true;
        if (low == "false" || low == "off" || low == "no" || low == "0") return false;
        return std::nullopt;
    });

    // string → int
    registerConverter<std::string, int>([](const std::string& s) -> std::optional<int> {
        try { return std::stoi(s); }
        catch (...) { return std::nullopt; }
    });

    // string → unsigned int
    registerConverter<std::string, unsigned int>([](const std::string& s) -> std::optional<unsigned int> {
        try { return static_cast<unsigned int>(std::stoul(s)); }
        catch (...) { return std::nullopt; }
    });

    // string → float
    registerConverter<std::string, float>([](const std::string& s) -> std::optional<float> {
        try { return std::stof(s); }
        catch (...) { return std::nullopt; }
    });

    // string → double
    registerConverter<std::string, double>([](const std::string& s) -> std::optional<double> {
        try { return std::stod(s); }
        catch (...) { return std::nullopt; }
    });

    // string → unsigned long (Pixel)
    registerConverter<std::string, unsigned long>([](const std::string& s) -> std::optional<unsigned long> {
        // Named colors: delegate to X11
        auto& app = Application::instance();
        auto* dpy = app.display();
        if (!dpy) {
            try { return std::stoul(s, nullptr, 0); }
            catch (...) { return std::nullopt; }
        }

        XColor exact, closest;
        if (XAllocNamedColor(dpy, DefaultColormap(dpy, app.screen()),
                             s.c_str(), &closest, &exact)) {
            return closest.pixel;
        }
        // Try hex "#RRGGBB"
        try { return std::stoul(s, nullptr, 0); }
        catch (...) { return std::nullopt; }
    });

    // int → string
    registerConverter<int, std::string>([](const int& v) -> std::optional<std::string> {
        return std::to_string(v);
    });

    // bool → string
    registerConverter<bool, std::string>([](const bool& v) -> std::optional<std::string> {
        return v ? std::string("true") : std::string("false");
    });
}

// ── ResourceDatabase ─────────────────────────────────────────────────

ResourceDatabase& ResourceDatabase::instance() {
    static ResourceDatabase db;
    return db;
}

void ResourceDatabase::put(const std::string& qualifiedName, const ResourceValue& value) {
    // Count specificity: number of non-wildcard components
    int spec = 0;
    for (char c : qualifiedName) {
        if (c == '.') spec += 2;
        else if (c == '*') spec += 1;
    }
    entries_.push_back({qualifiedName, value, spec});
}

int ResourceDatabase::matchSpecificity(const std::string& pattern,
                                        const std::string& widgetPath,
                                        const std::string& resourceName) const {
    // Simple pattern matching:
    // "*.resource" matches any path + resource
    // "app.widget.resource" matches exact path
    // "app*resource" matches with wildcard
    std::string full = widgetPath + "." + resourceName;

    // Exact match
    if (pattern == full) return 1000;

    // Ends with .resourceName
    std::string suffix = "." + resourceName;
    if (pattern.size() > suffix.size() &&
        pattern.substr(pattern.size() - suffix.size()) == suffix) {

        std::string prefix = pattern.substr(0, pattern.size() - suffix.size());

        // "*" prefix matches anything
        if (prefix == "*") return 100;

        // Check if prefix matches start of widgetPath with * as glob
        if (prefix.back() == '*') {
            std::string pfx = prefix.substr(0, prefix.size() - 1);
            if (widgetPath.find(pfx) == 0) return 500;
        }

        if (widgetPath.find(prefix) != std::string::npos) return 200;
    }

    // Wildcard resource match: "*resourceName"
    if (pattern == "*" + resourceName) return 50;

    return -1;  // no match
}

std::optional<ResourceValue> ResourceDatabase::lookup(
    const std::string& widgetPath,
    const std::string& resourceName) const
{
    int bestSpec = -1;
    const Entry* bestEntry = nullptr;

    for (const auto& entry : entries_) {
        int spec = matchSpecificity(entry.pattern, widgetPath, resourceName);
        if (spec > bestSpec) {
            bestSpec = spec;
            bestEntry = &entry;
        }
    }

    if (bestEntry) return bestEntry->value;
    return std::nullopt;
}

bool ResourceDatabase::loadFromString(const std::string& data) {
    std::istringstream stream(data);
    std::string line;

    while (std::getline(stream, line)) {
        // Skip comments and blank lines
        if (line.empty() || line[0] == '!' || line[0] == '#') continue;

        // Handle line continuation
        while (!line.empty() && line.back() == '\\') {
            line.pop_back();
            std::string next;
            if (!std::getline(stream, next)) break;
            line += next;
        }

        // Parse "pattern: value"
        auto colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string pattern = line.substr(0, colon);
        std::string value = line.substr(colon + 1);

        // Trim whitespace
        auto trim = [](std::string& s) {
            while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
            while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
        };
        trim(pattern);
        trim(value);

        if (!pattern.empty()) {
            put(pattern, ResourceValue{value});
        }
    }
    return true;
}

bool ResourceDatabase::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    return loadFromString(content);
}

bool ResourceDatabase::loadFromDisplay() {
    auto* dpy = Application::instance().display();
    if (!dpy) return false;

    char* resourceString = XResourceManagerString(dpy);
    if (resourceString) {
        return loadFromString(resourceString);
    }
    return false;
}

void ResourceDatabase::clear() {
    entries_.clear();
}

// ── Resource (per-widget bag) ────────────────────────────────────────

void Resource::applyDefaults(const std::string& widgetPath,
                              const std::vector<ResourceSpec>& specs) {
    auto& db = ResourceDatabase::instance();
    auto& conv = ResourceConverter::instance();

    for (const auto& spec : specs) {
        if (has(spec.name)) continue; // already explicitly set

        auto dbValue = db.lookup(widgetPath, spec.name);
        if (!dbValue) dbValue = db.lookup(widgetPath, spec.className);

        if (dbValue) {
            // Try to convert to target type if needed
            values_[spec.name] = *dbValue;
        } else {
            // Use default from spec
            values_[spec.name] = spec.defaultValue;
        }
    }
}

void Resource::setFromStrings(
    const std::unordered_map<std::string, std::string>& values,
    const std::vector<ResourceSpec>& specs)
{
    auto& conv = ResourceConverter::instance();

    for (const auto& [name, strVal] : values) {
        // Find matching spec
        for (const auto& spec : specs) {
            if (spec.name == name || spec.className == name) {
                auto result = conv.convert(
                    typeid(std::string), spec.type, ResourceValue{strVal});
                if (result) {
                    values_[spec.name] = *result;
                } else {
                    values_[spec.name] = strVal;
                }
                break;
            }
        }
    }
}

} // namespace motif
