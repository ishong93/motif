#include <motif/core/CompoundString.h>
#include <motif/core/FontRenderer.h>
#include <X11/Xlib.h>
#include <algorithm>
#include <cstring>
#include <numeric>
#include <sstream>

namespace motif {

// ── CompoundString ───────────────────────────────────────────────────

CompoundString::CompoundString(const std::string& text) {
    if (!text.empty()) {
        segments_.push_back({text, "default", StringDirection::LeftToRight, false});
    }
}

CompoundString::CompoundString(const std::string& text, const std::string& fontTag) {
    if (!text.empty()) {
        segments_.push_back({text, fontTag, StringDirection::LeftToRight, false});
    }
}

CompoundString CompoundString::create(const std::string& text) {
    return CompoundString(text);
}

CompoundString CompoundString::create(const std::string& text, const std::string& fontTag) {
    return CompoundString(text, fontTag);
}

void CompoundString::append(const std::string& text) {
    segments_.push_back({text, "default", StringDirection::LeftToRight, false});
}

void CompoundString::append(const std::string& text, const std::string& fontTag) {
    segments_.push_back({text, fontTag, StringDirection::LeftToRight, false});
}

void CompoundString::appendSeparator() {
    if (!segments_.empty()) {
        segments_.back().separator = true;
    } else {
        segments_.push_back({"", "default", StringDirection::LeftToRight, true});
    }
}

void CompoundString::appendSegment(const StringSegment& seg) {
    segments_.push_back(seg);
}

CompoundString& CompoundString::operator+=(const CompoundString& other) {
    segments_.insert(segments_.end(), other.segments_.begin(), other.segments_.end());
    return *this;
}

CompoundString CompoundString::operator+(const CompoundString& other) const {
    CompoundString result = *this;
    result += other;
    return result;
}

bool CompoundString::empty() const {
    return segments_.empty() ||
           std::all_of(segments_.begin(), segments_.end(),
                       [](const StringSegment& s) { return s.text.empty() && !s.separator; });
}

std::string CompoundString::toUtf8() const {
    std::string result;
    for (const auto& seg : segments_) {
        result += seg.text;
        if (seg.separator) result += '\n';
    }
    return result;
}

bool CompoundString::operator==(const CompoundString& other) const {
    if (segments_.size() != other.segments_.size()) return false;
    for (size_t i = 0; i < segments_.size(); ++i) {
        if (segments_[i].text != other.segments_[i].text) return false;
        if (segments_[i].fontTag != other.segments_[i].fontTag) return false;
        if (segments_[i].direction != other.segments_[i].direction) return false;
        if (segments_[i].separator != other.segments_[i].separator) return false;
    }
    return true;
}

// Helper: get XFontStruct for a segment from the RenderTable
static XFontStruct* fontForSegment(const StringSegment& seg, const RenderTable& rt,
                                    Display* dpy) {
    auto* rend = rt.findRendition(seg.fontTag);
    if (rend && rend->fontStruct) {
        return static_cast<XFontStruct*>(rend->fontStruct);
    }
    // Fallback to "default" rendition
    rend = rt.findRendition("default");
    if (rend && rend->fontStruct) {
        return static_cast<XFontStruct*>(rend->fontStruct);
    }
    // Last resort: default GC font
    return XQueryFont(dpy, XGContextFromGC(DefaultGC(dpy, DefaultScreen(dpy))));
}

CompoundString::Extent CompoundString::measure(void* display, const RenderTable& rt) const {
    auto* dpy = static_cast<Display*>(display);
    if (!dpy) return {0, 0};

    int maxW = 0;
    int totalH = 0;
    int lineW = 0;
    int lineH = 0;

    for (const auto& seg : segments_) {
        XFontStruct* font = fontForSegment(seg, rt, dpy);
        if (!font) continue;

        int segH = font->ascent + font->descent;
        lineH = std::max(lineH, segH);
        lineW += XTextWidth(font, seg.text.c_str(), static_cast<int>(seg.text.size()));

        if (seg.separator) {
            maxW = std::max(maxW, lineW);
            totalH += lineH;
            lineW = 0;
            lineH = 0;
        }
    }
    // Last line
    maxW = std::max(maxW, lineW);
    totalH += lineH;

    return {maxW, totalH};
}

CompoundString::Extent CompoundString::measureWithGC(void* display, void* gc) const {
    auto* dpy = static_cast<Display*>(display);
    auto xgc = static_cast<GC>(gc);
    if (!dpy || !xgc) return {0, 0};

    XFontStruct* font = XQueryFont(dpy, XGContextFromGC(xgc));
    if (!font) return {0, 0};

    int lineH = font->ascent + font->descent;
    int maxW = 0;
    int totalH = lineH;
    int lineW = 0;

    for (const auto& seg : segments_) {
        int tw = XTextWidth(font, seg.text.c_str(), static_cast<int>(seg.text.size()));
        lineW += tw;
        if (seg.separator) {
            maxW = std::max(maxW, lineW);
            lineW = 0;
            totalH += lineH;
        }
    }
    maxW = std::max(maxW, lineW);

    XFreeFontInfo(nullptr, font, 0);
    return {maxW, totalH};
}

void CompoundString::draw(void* display, unsigned long window, void* gc,
                           const RenderTable& rt, int x, int y) const {
    auto* dpy = static_cast<Display*>(display);
    auto xgc = static_cast<GC>(gc);
    if (!dpy || !xgc) return;

    int cx = x;
    int cy = y;

    // First pass: compute first line ascent for baseline
    int firstAscent = 0;
    for (const auto& seg : segments_) {
        XFontStruct* font = fontForSegment(seg, rt, dpy);
        if (font) { firstAscent = std::max(firstAscent, font->ascent); }
        if (seg.separator) break;
    }
    cy += firstAscent;

    for (const auto& seg : segments_) {
        XFontStruct* font = fontForSegment(seg, rt, dpy);
        if (!font) continue;

        if (font->fid) XSetFont(dpy, xgc, font->fid);

        if (seg.direction == StringDirection::RightToLeft) {
            int tw = XTextWidth(font, seg.text.c_str(), static_cast<int>(seg.text.size()));
            XDrawString(dpy, static_cast<Window>(window), xgc,
                        cx - tw, cy,
                        seg.text.c_str(), static_cast<int>(seg.text.size()));
            cx -= tw;
        } else {
            XDrawString(dpy, static_cast<Window>(window), xgc,
                        cx, cy,
                        seg.text.c_str(), static_cast<int>(seg.text.size()));
            cx += XTextWidth(font, seg.text.c_str(), static_cast<int>(seg.text.size()));
        }

        if (seg.separator) {
            cx = x;
            cy += font->ascent + font->descent;
        }
    }
}

void CompoundString::drawWithGC(void* display, unsigned long window, void* gc,
                                 int x, int y) const {
    auto* dpy = static_cast<Display*>(display);
    auto xgc = static_cast<GC>(gc);
    if (!dpy || !xgc) return;

    XFontStruct* font = XQueryFont(dpy, XGContextFromGC(xgc));
    if (!font) return;

    int lineH = font->ascent + font->descent;
    int cx = x;
    int cy = y + font->ascent;

    for (const auto& seg : segments_) {
        if (seg.direction == StringDirection::RightToLeft) {
            int tw = XTextWidth(font, seg.text.c_str(), static_cast<int>(seg.text.size()));
            XDrawString(dpy, static_cast<Window>(window), xgc,
                        cx - tw, cy,
                        seg.text.c_str(), static_cast<int>(seg.text.size()));
            cx -= tw;
        } else {
            XDrawString(dpy, static_cast<Window>(window), xgc,
                        cx, cy,
                        seg.text.c_str(), static_cast<int>(seg.text.size()));
            cx += XTextWidth(font, seg.text.c_str(), static_cast<int>(seg.text.size()));
        }

        if (seg.separator) {
            cx = x;
            cy += lineH;
        }
    }

    XFreeFontInfo(nullptr, font, 0);
}

// ── Serialization ────────────────────────────────────────────────────
// Binary format (simplified):
//   [4 bytes: segment count]
//   Per segment:
//     [4 bytes: text length] [text bytes]
//     [4 bytes: fontTag length] [fontTag bytes]
//     [1 byte: direction]
//     [1 byte: separator flag]

static void writeU32(std::vector<uint8_t>& buf, uint32_t val) {
    buf.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
    buf.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
    buf.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
    buf.push_back(static_cast<uint8_t>(val & 0xFF));
}

static uint32_t readU32(const uint8_t*& p) {
    uint32_t val = (static_cast<uint32_t>(p[0]) << 24) |
                   (static_cast<uint32_t>(p[1]) << 16) |
                   (static_cast<uint32_t>(p[2]) << 8) |
                    static_cast<uint32_t>(p[3]);
    p += 4;
    return val;
}

std::vector<uint8_t> CompoundString::serialize() const {
    std::vector<uint8_t> buf;
    writeU32(buf, static_cast<uint32_t>(segments_.size()));

    for (const auto& seg : segments_) {
        writeU32(buf, static_cast<uint32_t>(seg.text.size()));
        buf.insert(buf.end(), seg.text.begin(), seg.text.end());

        writeU32(buf, static_cast<uint32_t>(seg.fontTag.size()));
        buf.insert(buf.end(), seg.fontTag.begin(), seg.fontTag.end());

        buf.push_back(static_cast<uint8_t>(seg.direction));
        buf.push_back(seg.separator ? 1 : 0);
    }
    return buf;
}

CompoundString CompoundString::deserialize(const uint8_t* data, size_t size) {
    CompoundString result;
    if (!data || size < 4) return result;

    const uint8_t* p = data;
    const uint8_t* end = data + size;

    uint32_t count = readU32(p);

    for (uint32_t i = 0; i < count && p < end; ++i) {
        StringSegment seg;

        if (p + 4 > end) break;
        uint32_t textLen = readU32(p);
        if (p + textLen > end) break;
        seg.text.assign(reinterpret_cast<const char*>(p), textLen);
        p += textLen;

        if (p + 4 > end) break;
        uint32_t tagLen = readU32(p);
        if (p + tagLen > end) break;
        seg.fontTag.assign(reinterpret_cast<const char*>(p), tagLen);
        p += tagLen;

        if (p + 2 > end) break;
        seg.direction = static_cast<StringDirection>(*p++);
        seg.separator = (*p++ != 0);

        result.appendSegment(seg);
    }
    return result;
}

// ── RenderTable ──────────────────────────────────────────────────────

void RenderTable::addRendition(const Rendition& r) {
    // Replace existing with same tag
    for (auto& existing : renditions_) {
        if (existing.tag == r.tag) {
            existing = r;
            return;
        }
    }
    renditions_.push_back(r);
}

void RenderTable::removeRendition(const std::string& tag) {
    renditions_.erase(
        std::remove_if(renditions_.begin(), renditions_.end(),
                        [&](const Rendition& r) { return r.tag == tag; }),
        renditions_.end());
}

const RenderTable::Rendition* RenderTable::findRendition(const std::string& tag) const {
    for (const auto& r : renditions_) {
        if (r.tag == tag) return &r;
    }
    return nullptr;
}

void RenderTable::realize(void* display) {
    auto* dpy = static_cast<Display*>(display);
    if (!dpy) return;

    auto& renderer = motif::core::FontRenderer::instance();

    for (auto& r : renditions_) {
        if (r.fontHandle || r.fontStruct) continue; // already loaded

        // Try FontRenderer first (supports Xft when available)
        if (r.fontName.empty() || r.fontName == "default") {
            r.fontHandle = renderer.defaultFont();
        } else {
            r.fontHandle = renderer.loadFont(r.fontName);
        }

        // Keep fontStruct populated for backward compatibility
        if (r.fontHandle && r.fontHandle->xFontStruct()) {
            r.fontStruct = r.fontHandle->xFontStruct();
        } else if (!r.fontHandle) {
            // Direct fallback if FontRenderer not initialized
            if (r.fontName.empty() || r.fontName == "default") {
                r.fontStruct = XQueryFont(dpy, XGContextFromGC(DefaultGC(dpy, DefaultScreen(dpy))));
            } else {
                r.fontStruct = XLoadQueryFont(dpy, r.fontName.c_str());
            }
        }
    }
}

void RenderTable::free(void* display) {
    auto* dpy = static_cast<Display*>(display);
    if (!dpy) return;

    for (auto& r : renditions_) {
        // FontRenderer-managed fonts are ref-counted; just release handle
        r.fontHandle.reset();

        // Only free fontStruct if not managed by FontRenderer
        if (r.fontStruct && r.fontName != "default" && !r.fontHandle) {
            XFreeFont(dpy, static_cast<XFontStruct*>(r.fontStruct));
        }
        r.fontStruct = nullptr;
    }
}

RenderTable RenderTable::defaultTable(void* display) {
    RenderTable table;
    Rendition def;
    def.tag = "default";
    def.fontName = "default";
    table.addRendition(def);
    table.realize(display);
    return table;
}

} // namespace motif
