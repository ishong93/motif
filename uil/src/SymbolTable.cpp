#include <motif/uil/SymbolTable.h>
#include <algorithm>

namespace motif::uil {

SymbolTable::SymbolTable() {
    registerBuiltinClasses();
}

void SymbolTable::registerBuiltinClasses() {
    // All standard Motif widget classes
    const char* classes[] = {
        "XmArrowButton", "XmBulletinBoard", "XmCascadeButton",
        "XmCommand", "XmDialogShell", "XmDrawingArea",
        "XmDrawnButton", "XmFileSelectionBox", "XmForm",
        "XmFrame", "XmLabel", "XmLabelGadget",
        "XmList", "XmMainWindow", "XmMenuBar",
        "XmMenuShell", "XmMessageBox", "XmNotebook",
        "XmOptionMenu", "XmPanedWindow", "XmPopupMenu",
        "XmPulldownMenu", "XmPushButton", "XmPushButtonGadget",
        "XmRadioBox", "XmRowColumn", "XmScale",
        "XmScrollBar", "XmScrolledList", "XmScrolledText",
        "XmScrolledWindow", "XmSelectionBox", "XmSeparator",
        "XmSeparatorGadget", "XmSpinBox", "XmTabStack",
        "XmText", "XmTextField", "XmToggleButton",
        "XmToggleButtonGadget", "XmContainer", "XmComboBox",
        "XmIconGadget",
    };
    for (auto* cls : classes) {
        widgetClasses_[cls] = true;
    }
}

bool SymbolTable::define(const std::string& name, SymbolKind kind,
                         const std::string& typeName, bool exported, int line) {
    auto it = symbols_.find(name);
    if (it != symbols_.end() && it->second.defined) {
        return false; // already defined
    }

    Symbol sym;
    sym.name = name;
    sym.kind = kind;
    sym.typeName = typeName;
    sym.exported = exported;
    sym.defined = true;
    sym.definedLine = line;
    symbols_[name] = sym;
    return true;
}

Symbol* SymbolTable::lookup(const std::string& name) {
    auto it = symbols_.find(name);
    return (it != symbols_.end()) ? &it->second : nullptr;
}

const Symbol* SymbolTable::lookup(const std::string& name) const {
    auto it = symbols_.find(name);
    return (it != symbols_.end()) ? &it->second : nullptr;
}

bool SymbolTable::isDefined(const std::string& name) const {
    auto it = symbols_.find(name);
    return it != symbols_.end() && it->second.defined;
}

std::vector<std::string> SymbolTable::undefinedSymbols() const {
    std::vector<std::string> result;
    for (auto& [name, sym] : symbols_) {
        if (!sym.defined) {
            result.push_back(name);
        }
    }
    return result;
}

bool SymbolTable::isValidWidgetClass(const std::string& name) const {
    return widgetClasses_.count(name) > 0;
}

bool SymbolTable::isValidResourceName(const std::string& name) const {
    // Resource names start with "XmN" prefix
    return name.size() > 3 && name.substr(0, 3) == "XmN";
}

} // namespace motif::uil
