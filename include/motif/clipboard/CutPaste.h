#pragma once

#include <X11/Xlib.h>
#include <string>
#include <vector>
#include <functional>

namespace motif {

// X11 clipboard (CLIPBOARD / PRIMARY selection) integration.
namespace clipboard {

// Copy text to clipboard
bool copyText(Display* dpy, Window window, const std::string& text);

// Request paste from clipboard (async — callback fires when data arrives)
using PasteCallback = std::function<void(const std::string& data)>;
void pasteText(Display* dpy, Window window, PasteCallback callback);

// Lock/unlock clipboard for multi-item operations
bool lock(Display* dpy, Window window);
void unlock(Display* dpy, Window window);

// Store arbitrary data under a named format
bool store(Display* dpy, Window window,
           const std::string& format, const std::string& data);

// Retrieve data by format
bool retrieve(Display* dpy, Window window,
              const std::string& format, std::string& dataOut);

} // namespace clipboard
} // namespace motif
