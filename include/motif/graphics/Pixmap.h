#pragma once

#include <X11/Xlib.h>
#include <string>

namespace motif {

// RAII wrapper for X11 Pixmap.
class PixmapWrapper {
public:
    PixmapWrapper() = default;
    PixmapWrapper(Display* dpy, ::Pixmap pixmap, int width, int height);
    ~PixmapWrapper();

    // Move-only
    PixmapWrapper(PixmapWrapper&& other) noexcept;
    PixmapWrapper& operator=(PixmapWrapper&& other) noexcept;
    PixmapWrapper(const PixmapWrapper&) = delete;
    PixmapWrapper& operator=(const PixmapWrapper&) = delete;

    ::Pixmap pixmap() const { return pixmap_; }
    int width() const { return width_; }
    int height() const { return height_; }
    bool valid() const { return pixmap_ != None; }

    // Create from XBM data
    static PixmapWrapper fromBitmapData(Display* dpy, Drawable d,
                                         const unsigned char* data,
                                         int width, int height);

    // Create from file
    static PixmapWrapper fromFile(Display* dpy, Drawable d,
                                   const std::string& path);

private:
    Display* dpy_ = nullptr;
    ::Pixmap pixmap_ = None;
    int width_  = 0;
    int height_ = 0;
};

} // namespace motif
