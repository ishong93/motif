#pragma once

#include <X11/Xlib.h>
#include <string>
#include <unordered_map>
#include <memory>

// Xft/Fontconfig support (compile-time optional)
#ifdef HAVE_XFT
#include <X11/Xft/Xft.h>
#endif

namespace motif::core {

/// Font metrics returned by FontRenderer
struct FontMetrics {
    int ascent = 0;
    int descent = 0;
    int height = 0;      // ascent + descent
    int maxAdvance = 0;
};

/// Text extent (bounding box) for a string
struct TextExtent {
    int width = 0;
    int height = 0;
};

/// Abstract font handle — wraps either XftFont* or XFontStruct*
class Font {
public:
    virtual ~Font() = default;
    virtual FontMetrics metrics() const = 0;
    virtual TextExtent textExtent(const std::string& text) const = 0;
    virtual int textWidth(const char* str, int len) const = 0;
    virtual bool isXft() const = 0;

#ifdef HAVE_XFT
    virtual XftFont* xftFont() const { return nullptr; }
#endif
    virtual XFontStruct* xFontStruct() const { return nullptr; }
};

/// X11 core font wrapper
class CoreFont : public Font {
public:
    CoreFont(Display* dpy, XFontStruct* fs);
    ~CoreFont() override;

    FontMetrics metrics() const override;
    TextExtent textExtent(const std::string& text) const override;
    int textWidth(const char* str, int len) const override;
    bool isXft() const override { return false; }
    XFontStruct* xFontStruct() const override { return font_; }

private:
    Display* dpy_;
    XFontStruct* font_;
    bool owned_ = true;
};

#ifdef HAVE_XFT
/// Xft font wrapper (anti-aliased)
class XftFontWrap : public Font {
public:
    XftFontWrap(Display* dpy, int screen, XftFont* font);
    ~XftFontWrap() override;

    FontMetrics metrics() const override;
    TextExtent textExtent(const std::string& text) const override;
    int textWidth(const char* str, int len) const override;
    bool isXft() const override { return true; }
    XftFont* xftFont() const override { return font_; }

private:
    Display* dpy_;
    int screen_;
    XftFont* font_;
};
#endif

/// Central font rendering engine.
/// Loads fonts via Xft/Fontconfig when available, falls back to X11 core fonts.
/// Caches loaded fonts by name for efficient reuse.
class FontRenderer {
public:
    /// Get singleton instance
    static FontRenderer& instance();

    /// Initialize with display (must be called before loading fonts)
    void initialize(Display* dpy, int screen);

    /// Load a font by name.
    /// Xft names: "Sans-12", "Monospace-10:bold", "DejaVu Sans Mono-11"
    /// X11 names: "-*-fixed-medium-r-*-*-14-*-*-*-*-*-*-*", "fixed"
    /// Returns shared pointer (cached). Returns fallback if not found.
    std::shared_ptr<Font> loadFont(const std::string& name);

    /// Get the default fallback font
    std::shared_ptr<Font> defaultFont();

    /// Draw text using the appropriate backend
    void drawText(Drawable drawable, GC gc, const Font& font,
                  int x, int y, const std::string& text,
                  unsigned long color);

    /// Draw text with explicit length
    void drawText(Drawable drawable, GC gc, const Font& font,
                  int x, int y, const char* str, int len,
                  unsigned long color);

    /// Check if Xft rendering is available
    bool hasXft() const;

    /// Flush font cache
    void clearCache();

    /// Cleanup (call before display close)
    void shutdown();

private:
    FontRenderer() = default;
    ~FontRenderer() = default;
    FontRenderer(const FontRenderer&) = delete;
    FontRenderer& operator=(const FontRenderer&) = delete;

    std::shared_ptr<Font> loadXftFont(const std::string& name);
    std::shared_ptr<Font> loadCoreFont(const std::string& name);

    Display* dpy_ = nullptr;
    int screen_ = 0;
    bool initialized_ = false;

    std::unordered_map<std::string, std::shared_ptr<Font>> cache_;
    std::shared_ptr<Font> defaultFont_;

#ifdef HAVE_XFT
    XftDraw* xftDraw_ = nullptr;
    Drawable xftDrawable_ = None;
#endif
};

} // namespace motif::core
