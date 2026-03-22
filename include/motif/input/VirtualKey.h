#pragma once

#include <X11/Xlib.h>
#include <string>

namespace motif {

// Maps between X11 KeySyms and virtual key names (osfLeft, osfRight, etc.)
namespace virtualkey {

// Standard Motif virtual keysym names
constexpr const char* osfActivate   = "osfActivate";
constexpr const char* osfCancel     = "osfCancel";
constexpr const char* osfHelp       = "osfHelp";
constexpr const char* osfMenuBar    = "osfMenuBar";
constexpr const char* osfLeft       = "osfLeft";
constexpr const char* osfRight      = "osfRight";
constexpr const char* osfUp         = "osfUp";
constexpr const char* osfDown       = "osfDown";
constexpr const char* osfBeginLine  = "osfBeginLine";
constexpr const char* osfEndLine    = "osfEndLine";
constexpr const char* osfPageUp     = "osfPageUp";
constexpr const char* osfPageDown   = "osfPageDown";
constexpr const char* osfBackSpace  = "osfBackSpace";
constexpr const char* osfDelete     = "osfDelete";
constexpr const char* osfInsert     = "osfInsert";
constexpr const char* osfUndo       = "osfUndo";
constexpr const char* osfCut        = "osfCut";
constexpr const char* osfCopy       = "osfCopy";
constexpr const char* osfPaste      = "osfPaste";

// Resolve an X KeySym to a virtual key name (or empty string)
std::string keysymToVirtual(KeySym keysym);

// Resolve a virtual key name to an X KeySym (or NoSymbol)
KeySym virtualToKeysym(const std::string& virtualName);

} // namespace virtualkey
} // namespace motif
