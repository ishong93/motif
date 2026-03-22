#include <motif/graphics/ImageCache.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>

namespace motif {

ImageCache& ImageCache::instance() {
    static ImageCache cache;
    return cache;
}

ImageCache::~ImageCache() {
    clear();
}

const ImageCache::ImageEntry* ImageCache::loadImage(const std::string& name,
                                                     const std::string& path) {
    auto it = cache_.find(name);
    if (it != cache_.end()) return &it->second;

    auto& app = Application::instance();
    auto* dpy = app.display();
    if (!dpy) return nullptr;

    // Try XPM format
    XpmAttributes xpmAttr;
    xpmAttr.valuemask = 0;
    Pixmap pixmap = None, mask = None;

    int status = XpmReadFileToPixmap(dpy, app.rootWindow(),
                                      const_cast<char*>(path.c_str()),
                                      &pixmap, &mask, &xpmAttr);
    if (status == XpmSuccess) {
        ImageEntry entry;
        entry.pixmap = pixmap;
        entry.width = xpmAttr.width;
        entry.height = xpmAttr.height;
        XpmFreeAttributes(&xpmAttr);
        if (mask != None) XFreePixmap(dpy, mask);

        cache_[name] = entry;
        return &cache_[name];
    }

    return nullptr;
}

const ImageCache::ImageEntry* ImageCache::findImage(const std::string& name) const {
    auto it = cache_.find(name);
    return it != cache_.end() ? &it->second : nullptr;
}

void ImageCache::unloadImage(const std::string& name) {
    auto it = cache_.find(name);
    if (it == cache_.end()) return;

    auto* dpy = Application::instance().display();
    if (dpy && it->second.pixmap != None) {
        XFreePixmap(dpy, it->second.pixmap);
    }
    cache_.erase(it);
}

void ImageCache::clear() {
    auto* dpy = Application::instance().display();
    for (auto& [name, entry] : cache_) {
        if (dpy && entry.pixmap != None) {
            XFreePixmap(dpy, entry.pixmap);
        }
    }
    cache_.clear();
}

} // namespace motif
