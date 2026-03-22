#include <motif/input/VirtualKey.h>
#include <X11/keysym.h>
#include <unordered_map>

namespace motif {
namespace virtualkey {

static const std::unordered_map<KeySym, std::string>& keysymMap() {
    static const std::unordered_map<KeySym, std::string> map = {
        {XK_Return,    osfActivate},
        {XK_KP_Enter,  osfActivate},
        {XK_Escape,    osfCancel},
        {XK_F1,        osfHelp},
        {XK_F10,       osfMenuBar},
        {XK_Left,      osfLeft},
        {XK_Right,     osfRight},
        {XK_Up,        osfUp},
        {XK_Down,      osfDown},
        {XK_Home,      osfBeginLine},
        {XK_End,       osfEndLine},
        {XK_Page_Up,   osfPageUp},
        {XK_Page_Down, osfPageDown},
        {XK_BackSpace, osfBackSpace},
        {XK_Delete,    osfDelete},
        {XK_Insert,    osfInsert},
    };
    return map;
}

static const std::unordered_map<std::string, KeySym>& reverseMap() {
    static std::unordered_map<std::string, KeySym> rmap;
    static bool built = false;
    if (!built) {
        for (auto& [ks, name] : keysymMap()) {
            rmap[name] = ks;
        }
        built = true;
    }
    return rmap;
}

std::string keysymToVirtual(KeySym keysym) {
    auto& m = keysymMap();
    auto it = m.find(keysym);
    return it != m.end() ? it->second : std::string{};
}

KeySym virtualToKeysym(const std::string& virtualName) {
    auto& m = reverseMap();
    auto it = m.find(virtualName);
    return it != m.end() ? it->second : NoSymbol;
}

} // namespace virtualkey
} // namespace motif
