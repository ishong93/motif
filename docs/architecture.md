# Motif C++ Architecture

## Project Structure

```
motif/
├── include/motif/           # Public headers
│   ├── core/                # Application, Widget, Shell, Resource, ...
│   ├── widgets/             # PushButton, Label, Text, List, Scale, ...
│   ├── containers/          # Form, RowColumn, Frame, PanedWindow, ...
│   ├── dialogs/             # MessageBox, FileSelectionBox, Command, ...
│   ├── menus/               # MenuBar, MenuShell, PopupMenu
│   ├── dnd/                 # DragContext, DragIcon, DropSiteManager
│   ├── graphics/            # Color, Draw, ImageCache, Pixmap
│   ├── input/               # KeyBinding, VirtualKey, Traversal
│   ├── clipboard/           # CutPaste, Transfer
│   ├── print/               # PrintContext (Cairo PDF/PS)
│   ├── mrm/                 # ResourceLoader (UID binary)
│   ├── Xm.h                # Convenience header (includes everything)
│   ├── Version.h            # Version macros and constexpr struct
│   └── Builder.h            # Fluent builder API
├── src/                     # Library implementation
├── wm/                      # Window manager (motif-wm)
│   ├── include/motif/wm/    # WM headers
│   └── src/                 # WM implementation
├── uil/                     # UIL compiler (motif-uil)
│   ├── include/motif/uil/   # UIL headers
│   └── src/                 # Lexer, Parser, Compiler
├── mrm/                     # MRM resource manager library
├── tests/                   # GoogleTest unit tests
├── examples/                # Example programs
├── resources/               # Default resource files
├── legacy/                  # Original OpenMotif source (reference)
├── cmake/                   # CMake modules and config templates
└── .github/workflows/       # CI/CD
```

## Core Design Principles

### 1. No Xt Intrinsics

The original Motif was built on top of the Xt (X Toolkit) Intrinsics
library. This rewrite eliminates Xt entirely:

- **Widget lifecycle** is managed by C++ constructors/destructors
- **Event dispatch** is done via direct Xlib event handling
- **Resource management** uses `std::unordered_map<std::string, std::any>`
- **Callbacks** use `std::function` instead of Xt callback lists

### 2. Direct X11/Xlib

All rendering and window management uses Xlib directly:

- Each Widget creates its own X window via `XCreateWindow()`
- Drawing uses `XDrawString()`, `XFillRectangle()`, etc.
- Optional Xft rendering for anti-aliased fonts
- Optional Cairo for print output

### 3. C++17 Features

- `std::variant` for expression values in UIL AST
- `std::optional` for nullable resource values
- `std::any` for type-erased resource storage
- `std::function` for callbacks
- Structured bindings for container iteration
- `if constexpr` for compile-time branching
- Inline variables for singletons

### 4. Compile-Time Optional Dependencies

| Feature | Library | Macro | Default |
|---------|---------|-------|---------|
| Anti-aliased fonts | Xft + Fontconfig | `HAVE_XFT` | ON |
| PDF/PS printing | Cairo | `HAVE_CAIRO` | ON |
| Accessibility | AT-SPI + D-Bus | `HAVE_ATSPI` | OFF |

## Module Architecture

### Widget Hierarchy

```
Widget (base)
├── Primitive (leaf widgets)
│   ├── Label
│   │   ├── PushButton
│   │   │   ├── ToggleButton
│   │   │   └── CascadeButton
│   │   ├── DrawnButton
│   │   └── ArrowButton
│   ├── Text / TextField
│   ├── List
│   ├── ScrollBar
│   ├── Scale
│   ├── Separator
│   └── ComboBox
├── Manager (container widgets)
│   ├── BulletinBoard
│   │   ├── Form
│   │   ├── SelectionBox
│   │   │   └── FileSelectionBox
│   │   ├── MessageBox
│   │   └── Command
│   ├── RowColumn
│   ├── Frame
│   ├── PanedWindow
│   ├── ScrolledWindow
│   │   └── MainWindow
│   ├── Notebook
│   ├── TabStack
│   └── Container
├── Gadget (lightweight — shares parent window)
└── Shell (top-level windows)
    └── DialogShell
```

### Singletons

| Class | Purpose |
|-------|---------|
| `Application` | Display connection, event loop |
| `FontRenderer` | Font loading and text rendering |
| `ThemeManager` | Active theme and change notification |
| `InputMethod` | XIM connection and IC management |
| `DropSiteManager` | DnD drop target registry |
| `HiDPI` | Display scale factor detection |
| `AccessibilityBridge` | AT-SPI widget registration |

### Event Flow

```
XNextEvent()
  → Application::dispatchEvent()
    → InputMethod::filterEvent()  (XIM filtering)
    → Widget::handleEvent()       (per-widget dispatch)
      → onButtonPress / onKeyPress / onExpose callbacks
      → Layout recalculation if geometry changed
```

### Resource System

Resources use a key-value store per widget:

```cpp
// Internal storage
std::unordered_map<std::string, std::any> resources_;

// Typed access
widget.set<int>("width", 300);
int w = widget.get<int>("width");
```

The `TypeRegistry` maps resource names to types for validation.

### Theme System

Three-level cascade (matching Motif resource precedence):

1. **Base style** — default colors/fonts for all widgets
2. **Class style** — per-widget-class overrides (e.g., all PushButtons)
3. **Instance style** — per-widget-instance overrides (by name)

Built-in themes: Classic, Modern, Dark, HighContrast.

### Drag and Drop

Two protocols:

1. **Internal DnD** — `DropSiteManager` tracks registered widgets,
   `DragContext` dispatches enter/leave/drop events locally
2. **XDND v5** — For inter-application drag-and-drop, using
   standard `XdndEnter/Position/Status/Leave/Drop/Finished` messages

Visual feedback: `DragIcon` (override-redirect window) follows cursor
with Accept/Reject state indicators.

### UIL Compiler Pipeline

```
.uil source → Lexer → Parser → AST → Compiler → .uid binary (IDB format)
```

- **Lexer**: Tokenizes keywords, literals, operators, comments
- **Parser**: Recursive descent, produces `ModuleNode` AST
- **Compiler**: Evaluates constant expressions, serializes to IDB records
- **IDB format**: Record-based binary (4KB records, magic header, index)

### Window Manager (motif-wm)

```
WindowManager
├── EventHandler     — X event dispatch
├── Decoration       — Frame/title bar drawing
├── KeyFocus         — Focus stack and cycling
├── Menu             — Root and window menus
├── EWMH             — _NET_WM_* protocol
├── ConfigParser     — .mwmrc file parser
└── SessionManager   — XSMP session save/restore
```

Features:
- Reparenting WM with Motif-style decorations
- EWMH compliance (taskbars, pagers, system trays)
- Virtual desktops with `_NET_NUMBER_OF_DESKTOPS`
- Click-to-focus and focus-follows-mouse policies
- WM_DELETE_WINDOW protocol for graceful close
- Size hints (min/max/increment) enforcement

## Build System

CMake 3.20+ with:
- `FetchContent` for GoogleTest
- `pkg-config` for Xft, Fontconfig, Cairo
- Exported CMake config (`find_package(Motif)`)
- pkg-config file (`motif.pc`)
- Component options: `MOTIF_BUILD_TESTS`, `MOTIF_BUILD_WM`, etc.
