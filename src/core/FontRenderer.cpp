#include <motif/core/FontRenderer.h>
#include <motif/core/Log.h>

#include <X11/Xutil.h>
#include <cstring>

namespace motif::core {

// ── CoreFont ────────────────────────────────────────────────

CoreFont::CoreFont(Display* dpy, XFontStruct* fs)
    : dpy_(dpy), font_(fs) {}

CoreFont::~CoreFont() {
    if (font_ && dpy_ && owned_) {
        XFreeFont(dpy_, font_);
    }
}

FontMetrics CoreFont::metrics() const {
    FontMetrics m;
    if (font_) {
        m.ascent = font_->ascent;
        m.descent = font_->descent;
        m.height = font_->ascent + font_->descent;
        m.maxAdvance = font_->max_bounds.width;
    }
    return m;
}

TextExtent CoreFont::textExtent(const std::string& text) const {
    TextExtent ext;
    if (font_ && !text.empty()) {
        ext.width = XTextWidth(font_, text.c_str(), static_cast<int>(text.size()));
        ext.height = font_->ascent + font_->descent;
    }
    return ext;
}

int CoreFont::textWidth(const char* str, int len) const {
    if (!font_ || !str || len <= 0) return 0;
    return XTextWidth(font_, str, len);
}

// ── XftFontWrap ─────────────────────────────────────────────

#ifdef HAVE_XFT
XftFontWrap::XftFontWrap(Display* dpy, int screen, XftFont* font)
    : dpy_(dpy), screen_(screen), font_(font) {}

XftFontWrap::~XftFontWrap() {
    if (font_ && dpy_) {
        XftFontClose(dpy_, font_);
    }
}

FontMetrics XftFontWrap::metrics() const {
    FontMetrics m;
    if (font_) {
        m.ascent = font_->ascent;
        m.descent = font_->descent;
        m.height = font_->ascent + font_->descent;
        m.maxAdvance = font_->max_advance_width;
    }
    return m;
}

TextExtent XftFontWrap::textExtent(const std::string& text) const {
    TextExtent ext;
    if (font_ && !text.empty()) {
        XGlyphInfo info;
        XftTextExtentsUtf8(dpy_, font_,
                           reinterpret_cast<const FcChar8*>(text.c_str()),
                           static_cast<int>(text.size()), &info);
        ext.width = info.xOff;
        ext.height = font_->ascent + font_->descent;
    }
    return ext;
}

int XftFontWrap::textWidth(const char* str, int len) const {
    if (!font_ || !str || len <= 0) return 0;
    XGlyphInfo info;
    XftTextExtentsUtf8(dpy_, font_,
                       reinterpret_cast<const FcChar8*>(str), len, &info);
    return info.xOff;
}
#endif

// ── FontRenderer ────────────────────────────────────────────

FontRenderer& FontRenderer::instance() {
    static FontRenderer renderer;
    return renderer;
}

void FontRenderer::initialize(Display* dpy, int screen) {
    dpy_ = dpy;
    screen_ = screen;
    initialized_ = true;

    MOTIF_LOG_INFO << "FontRenderer initialized"
#ifdef HAVE_XFT
                   << " (Xft enabled)";
#else
                   << " (X11 core fonts)";
#endif
}

std::shared_ptr<Font> FontRenderer::loadFont(const std::string& name) {
    if (!initialized_) return nullptr;

    // Check cache first
    auto it = cache_.find(name);
    if (it != cache_.end()) return it->second;

    std::shared_ptr<Font> font;

#ifdef HAVE_XFT
    // Try Xft first for modern font names
    font = loadXftFont(name);
#endif

    // Fall back to X11 core fonts
    if (!font) {
        font = loadCoreFont(name);
    }

    if (font) {
        cache_[name] = font;
    } else {
        MOTIF_LOG_WARNING << "Font not found: " << name << ", using default";
        font = defaultFont();
    }

    return font;
}

std::shared_ptr<Font> FontRenderer::defaultFont() {
    if (defaultFont_) return defaultFont_;
    if (!initialized_) return nullptr;

    // Try common fallbacks
    static const char* fallbacks[] = {
#ifdef HAVE_XFT
        "Sans-11",
        "DejaVu Sans-11",
        "Liberation Sans-11",
#endif
        "fixed",
        "-*-fixed-medium-r-*-*-14-*-*-*-*-*-*-*",
        nullptr
    };

    for (const char** name = fallbacks; *name; ++name) {
        auto font = loadFont(*name);
        if (font) {
            defaultFont_ = font;
            return defaultFont_;
        }
    }

    // Last resort: load whatever "fixed" gives us
    XFontStruct* fs = XLoadQueryFont(dpy_, "fixed");
    if (fs) {
        defaultFont_ = std::make_shared<CoreFont>(dpy_, fs);
        cache_["fixed"] = defaultFont_;
    }

    return defaultFont_;
}

std::shared_ptr<Font> FontRenderer::loadXftFont(const std::string& name) {
#ifdef HAVE_XFT
    // Try opening as Xft font name (e.g., "Sans-12:bold")
    XftFont* xft = XftFontOpenName(dpy_, screen_, name.c_str());
    if (xft) {
        return std::make_shared<XftFontWrap>(dpy_, screen_, xft);
    }

    // Try as XLFD pattern via Xft
    xft = XftFontOpenXlfd(dpy_, screen_, name.c_str());
    if (xft) {
        return std::make_shared<XftFontWrap>(dpy_, screen_, xft);
    }
#else
    (void)name;
#endif
    return nullptr;
}

std::shared_ptr<Font> FontRenderer::loadCoreFont(const std::string& name) {
    XFontStruct* fs = XLoadQueryFont(dpy_, name.c_str());
    if (fs) {
        return std::make_shared<CoreFont>(dpy_, fs);
    }
    return nullptr;
}

void FontRenderer::drawText(Drawable drawable, GC gc, const Font& font,
                            int x, int y, const std::string& text,
                            unsigned long color) {
    drawText(drawable, gc, font, x, y, text.c_str(),
             static_cast<int>(text.size()), color);
}

void FontRenderer::drawText(Drawable drawable, GC gc, const Font& font,
                            int x, int y, const char* str, int len,
                            unsigned long color) {
    if (!str || len <= 0 || !initialized_) return;

#ifdef HAVE_XFT
    if (font.isXft()) {
        // Ensure we have an XftDraw for this drawable
        if (xftDrawable_ != drawable || !xftDraw_) {
            if (xftDraw_) XftDrawDestroy(xftDraw_);
            xftDraw_ = XftDrawCreate(dpy_, drawable,
                                     DefaultVisual(dpy_, screen_),
                                     DefaultColormap(dpy_, screen_));
            xftDrawable_ = drawable;
        }

        if (xftDraw_) {
            XftColor xftColor;
            XColor xc;
            xc.pixel = color;
            XQueryColor(dpy_, DefaultColormap(dpy_, screen_), &xc);
            xftColor.pixel = color;
            xftColor.color.red = xc.red;
            xftColor.color.green = xc.green;
            xftColor.color.blue = xc.blue;
            xftColor.color.alpha = 0xFFFF;

            XftDrawStringUtf8(xftDraw_, &xftColor, font.xftFont(),
                              x, y,
                              reinterpret_cast<const FcChar8*>(str), len);
        }
        return;
    }
#endif

    // X11 core font path
    if (font.xFontStruct()) {
        XSetFont(dpy_, gc, font.xFontStruct()->fid);
    }
    XSetForeground(dpy_, gc, color);
    XDrawString(dpy_, drawable, gc, x, y, str, len);
}

bool FontRenderer::hasXft() const {
#ifdef HAVE_XFT
    return true;
#else
    return false;
#endif
}

void FontRenderer::clearCache() {
    cache_.clear();
    defaultFont_.reset();
}

void FontRenderer::shutdown() {
#ifdef HAVE_XFT
    if (xftDraw_) {
        XftDrawDestroy(xftDraw_);
        xftDraw_ = nullptr;
        xftDrawable_ = None;
    }
#endif
    clearCache();
    initialized_ = false;
}

} // namespace motif::core
