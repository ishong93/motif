#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace motif::uil {

enum class SymbolKind {
    Object,
    Value,
    Procedure,
    WidgetClass,
    Identifier,
};

struct Symbol {
    std::string name;
    SymbolKind kind = SymbolKind::Object;
    std::string typeName;      // widget class, value type, etc.
    bool exported = false;
    bool defined = false;       // has been fully defined
    int definedLine = 0;
};

class SymbolTable {
public:
    SymbolTable();

    bool define(const std::string& name, SymbolKind kind,
                const std::string& typeName = {}, bool exported = false, int line = 0);
    Symbol* lookup(const std::string& name);
    const Symbol* lookup(const std::string& name) const;
    bool isDefined(const std::string& name) const;

    // Check for undefined forward references
    std::vector<std::string> undefinedSymbols() const;

    // Widget class validation
    bool isValidWidgetClass(const std::string& name) const;

    // Resource name validation
    bool isValidResourceName(const std::string& name) const;

private:
    void registerBuiltinClasses();

    std::unordered_map<std::string, Symbol> symbols_;
    std::unordered_map<std::string, bool> widgetClasses_;
};

} // namespace motif::uil
