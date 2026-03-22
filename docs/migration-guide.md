# Migration Guide: Legacy Motif (C/Xt) → Modern Motif (C++17)

This guide helps developers port existing Motif/Xt applications to the
modern C++17 API.

## Overview of Changes

| Legacy (C/Xt)                  | Modern (C++17)                        |
|-------------------------------|---------------------------------------|
| `Widget` (opaque Xt type)     | `motif::Widget` (C++ class)           |
| `XtAppInitialize()`          | `motif::Application::open()`          |
| `XtMainLoop()`               | `motif::Application::run()`           |
| `XtVaCreateManagedWidget()`  | Direct construction: `PushButton btn(&parent)` |
| `XtAddCallback()`            | `widget.addCallback("name", lambda)` |
| `XmString`                   | `motif::CompoundString` or `std::string` |
| `Arg` / `XtSetArg()`        | `widget.set<T>("resource", value)`    |
| `XtGetValues()` / `XtSetValues()` | `widget.get<T>("name")` / `widget.set()` |
| Xt Intrinsics event loop     | Direct Xlib event dispatch            |
| Motif resource files         | Same format, loaded via `core::Theme` |

## Quick Start

### Before (Legacy C)

```c
#include <Xm/Xm.h>
#include <Xm/PushB.h>

void button_cb(Widget w, XtPointer client, XtPointer call) {
    printf("Button pressed!\n");
}

int main(int argc, char* argv[]) {
    Widget toplevel, button;
    XtAppContext app;
    Arg args[10];
    int n;

    toplevel = XtAppInitialize(&app, "MyApp", NULL, 0,
                                &argc, argv, NULL, NULL, 0);

    n = 0;
    XtSetArg(args[n], XmNlabelString,
             XmStringCreateLocalized("Click Me")); n++;
    XtSetArg(args[n], XmNwidth, 200); n++;
    XtSetArg(args[n], XmNheight, 50); n++;

    button = XmCreatePushButton(toplevel, "button", args, n);
    XtManageChild(button);
    XtAddCallback(button, XmNactivateCallback, button_cb, NULL);

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app);
    return 0;
}
```

### After (Modern C++17)

```cpp
#include <motif/Xm.h>  // convenience header

int main() {
    motif::Application::open("MyApp");
    motif::Shell shell("MyApp", 200, 50);

    motif::PushButton button(&shell, "button");
    button.setLabel("Click Me");
    button.onActivate([] {
        printf("Button pressed!\n");
    });

    shell.realize();
    motif::Application::run();
    motif::Application::close();
}
```

## Concept Mapping

### Widget Creation

| Legacy | Modern |
|--------|--------|
| `XmCreatePushButton(parent, name, args, n)` | `PushButton btn(&parent, name)` |
| `XmCreateLabel(parent, name, args, n)` | `Label lbl(&parent, name)` |
| `XmCreateToggleButton(parent, name, args, n)` | `ToggleButton tog(&parent)` |
| `XmCreateText(parent, name, args, n)` | `Text text(&parent)` |
| `XmCreateScrollBar(parent, name, args, n)` | `ScrollBar sb(&parent)` |
| `XmCreateMenuBar(parent, name, args, n)` | `MenuBar mb(&parent)` |

### Containers

| Legacy | Modern |
|--------|--------|
| `XmCreateForm(parent, name, args, n)` | `Form form(&parent)` |
| `XmCreateRowColumn(parent, name, args, n)` | `RowColumn rc(&parent)` |
| `XmCreateBulletinBoard(parent, name, args, n)` | `BulletinBoard bb(&parent)` |
| `XmCreateMainWindow(parent, name, args, n)` | `MainWindow mw(&parent)` |
| `XmCreatePanedWindow(parent, name, args, n)` | `PanedWindow pw(&parent)` |
| `XmCreateFrame(parent, name, args, n)` | `Frame frame(&parent)` |

### Dialogs

| Legacy | Modern |
|--------|--------|
| `XmCreateMessageDialog(parent, name, args, n)` | `MessageBox::information(parent, title, msg)` |
| `XmCreateFileSelectionDialog(parent, name, args, n)` | `FileSelectionBox fsb(&parent)` |

### Resources

```c
// Legacy: Arg arrays
Arg args[2];
XtSetArg(args[0], XmNwidth, 300);
XtSetArg(args[1], XmNheight, 200);
XtSetValues(widget, args, 2);
```

```cpp
// Modern: typed setter
widget.set<int>("width", 300);
widget.set<int>("height", 200);
// Or direct methods
widget.setPosition(0, 0, 300, 200);
```

### Callbacks

```c
// Legacy: function pointer + client data
void my_callback(Widget w, XtPointer client_data, XtPointer call_data) {
    char* msg = (char*)client_data;
    printf("%s\n", msg);
}
XtAddCallback(button, XmNactivateCallback, my_callback, "hello");
```

```cpp
// Modern: lambda or std::function
std::string msg = "hello";
button.onActivate([msg] {
    printf("%s\n", msg.c_str());
});
```

### Compound Strings

```c
// Legacy
XmString str = XmStringCreateLocalized("Hello");
XtVaSetValues(label, XmNlabelString, str, NULL);
XmStringFree(str);
```

```cpp
// Modern — automatic lifetime
label.setLabel("Hello");
// Or explicit CompoundString for complex formatting:
CompoundString cs;
cs.append("Hello", "bold");
cs.appendSeparator();
cs.append("World", "normal");
label.setCompoundString(cs);
```

### Event Handling

```c
// Legacy: Xt translation tables
static char translations[] = "<Btn1Down>: my_action()";
XtOverrideTranslations(widget, XtParseTranslationTable(translations));
```

```cpp
// Modern: direct event handlers
widget.onButtonPress([](int button, int x, int y) {
    // handle button press
});
widget.onKeyPress([](KeySym key, unsigned int modifiers) {
    // handle key press
});
```

## Builder Pattern (New)

For rapid prototyping, use the builder API:

```cpp
#include <motif/Builder.h>

int main() {
    motif::QuickApp app("My App", 400, 300);

    auto [nameField, _] = app.builder.labeledField("Name:");
    auto& okBtn = app.builder.button("OK", [&] {
        auto name = nameField.text();
        printf("Hello, %s!\n", name.c_str());
    });

    app.run();
}
```

## Theme System (New)

```cpp
// Apply a built-in theme
motif::core::ThemeManager::instance().setTheme(
    motif::core::Theme::motifDark()
);

// Listen for theme changes
motif::core::ThemeManager::instance().addChangeCallback([](const auto& theme) {
    // update custom widgets
});
```

## Font Rendering (New)

Modern Motif uses Xft for anti-aliased fonts when available:

```cpp
auto& renderer = motif::core::FontRenderer::instance();
auto font = renderer.loadFont("Sans", 12);
if (font) {
    renderer.drawString(drawable, gc, *font, 10, 20, "Hello");
}
```

## Drag and Drop (New)

```cpp
// Source
DragContext drag(&sourceWidget, {"text/plain", "UTF8_STRING"});
drag.setConvertCallback([](const std::string& type, std::string& data) {
    data = "dragged text";
    return true;
});
drag.startDrag(x, y);

// Target
DropSiteManager::instance().registerDropSite(&targetWidget, {
    .importTargets = {"text/plain"},
    .dropCb = [](DragContext& ctx, const std::string& data) {
        printf("Dropped: %s\n", data.c_str());
        return true;
    }
});
```

## Build System

Replace autoconf/Imake with CMake:

```cmake
find_package(Motif REQUIRED)
target_link_libraries(myapp PRIVATE Motif::motif)
```

## What's Removed

- **Xt Intrinsics dependency**: All widget management is built-in
- **Motif DnD protocol**: Replaced by XDND (inter-client) + internal DnD
- **UIL binary format changes**: New IDB format (not backward compatible)
- **Gadget rendering in parents**: Gadgets now have their own X windows
