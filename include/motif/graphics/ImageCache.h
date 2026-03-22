#pragma once

#include <string>
#include <unordered_map>
#include <X11/Xlib.h>

namespace motif {

// Caches pixmaps/images by name to avoid reloading from disk.
class ImageCache {
public:
    static ImageCache& instance();

    struct ImageEntry {
        Pixmap pixmap = None;
        int width  = 0;
        int height = 0;
    };

    // Load an XBM/XPM from file (returns cached if already loaded)
    const ImageEntry* loadImage(const std::string& name, const std::string& path);

    // Look up a previously loaded image
    const ImageEntry* findImage(const std::string& name) const;

    // Remove from cache and free pixmap
    void unloadImage(const std::string& name);

    // Free all cached images
    void clear();

private:
    ImageCache() = default;
    ~ImageCache();

    std::unordered_map<std::string, ImageEntry> cache_;
};

} // namespace motif
