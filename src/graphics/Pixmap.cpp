#include <motif/graphics/Pixmap.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <utility>

namespace motif {

PixmapWrapper::PixmapWrapper(Display* dpy, ::Pixmap pixmap, int width, int height)
    : dpy_(dpy), pixmap_(pixmap), width_(width), height_(height) {}

PixmapWrapper::~PixmapWrapper() {
    if (dpy_ && pixmap_ != None) {
        XFreePixmap(dpy_, pixmap_);
    }
}

PixmapWrapper::PixmapWrapper(PixmapWrapper&& other) noexcept
    : dpy_(other.dpy_), pixmap_(other.pixmap_),
      width_(other.width_), height_(other.height_) {
    other.pixmap_ = None;
    other.dpy_ = nullptr;
}

PixmapWrapper& PixmapWrapper::operator=(PixmapWrapper&& other) noexcept {
    if (this != &other) {
        if (dpy_ && pixmap_ != None) XFreePixmap(dpy_, pixmap_);
        dpy_ = other.dpy_;
        pixmap_ = other.pixmap_;
        width_ = other.width_;
        height_ = other.height_;
        other.pixmap_ = None;
        other.dpy_ = nullptr;
    }
    return *this;
}

PixmapWrapper PixmapWrapper::fromBitmapData(Display* dpy, Drawable d,
                                              const unsigned char* data,
                                              int width, int height) {
    ::Pixmap pix = XCreateBitmapFromData(dpy, d,
                                          reinterpret_cast<const char*>(data),
                                          width, height);
    return PixmapWrapper(dpy, pix, width, height);
}

PixmapWrapper PixmapWrapper::fromFile(Display* dpy, Drawable d,
                                       const std::string& path) {
    unsigned int w, h;
    ::Pixmap pix;
    int xHot, yHot;

    int status = XReadBitmapFile(dpy, d, path.c_str(), &w, &h, &pix, &xHot, &yHot);
    if (status == BitmapSuccess) {
        return PixmapWrapper(dpy, pix, static_cast<int>(w), static_cast<int>(h));
    }
    return PixmapWrapper();
}

} // namespace motif
