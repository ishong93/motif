#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace motif::core { class Font; }

namespace motif {

// ── CompoundString ───────────────────────────────────────────────────
// Replaces XmString — Motif's compound string type that supported
// multi-segment, multi-directional, multi-font text.
//
// Redesigned as modern C++17 with UTF-8 throughout:
//   - No more manual charset tags (XmFONTLIST_DEFAULT_TAG)
//   - Segments carry font/direction/tab metadata
//   - Renderable via a single draw() call that queries the RenderTable

enum class StringDirection { LeftToRight, RightToLeft };

struct StringSegment {
    std::string text;                               // UTF-8 text
    std::string fontTag = "default";                // maps to RenderTable tag
    StringDirection direction = StringDirection::LeftToRight;
    bool separator = false;                         // line break after this segment
};

class RenderTable;

class CompoundString {
public:
    CompoundString() = default;
    explicit CompoundString(const std::string& text);
    CompoundString(const std::string& text, const std::string& fontTag);
    CompoundString(const CompoundString&) = default;
    CompoundString& operator=(const CompoundString&) = default;
    CompoundString(CompoundString&&) = default;
    CompoundString& operator=(CompoundString&&) = default;

    // Factory method (convenience)
    static CompoundString create(const std::string& text);
    static CompoundString create(const std::string& text, const std::string& fontTag);

    // ── Building ─────────────────────────────────────────────────
    void append(const std::string& text);
    void append(const std::string& text, const std::string& fontTag);
    void appendSeparator();
    void appendSegment(const StringSegment& seg);

    CompoundString& operator+=(const CompoundString& other);
    CompoundString operator+(const CompoundString& other) const;

    // ── Queries ──────────────────────────────────────────────────
    bool empty() const;
    int segmentCount() const { return static_cast<int>(segments_.size()); }
    const StringSegment& segment(int index) const { return segments_[index]; }
    const std::vector<StringSegment>& segments() const { return segments_; }

    // Flatten to plain UTF-8 (ignoring font tags and direction)
    std::string toUtf8() const;

    // Compare (like XmStringCompare)
    bool operator==(const CompoundString& other) const;
    bool operator!=(const CompoundString& other) const { return !(*this == other); }

    // ── Measurement ──────────────────────────────────────────────
    struct Extent {
        int width  = 0;
        int height = 0;
    };

    // Measure using a RenderTable for font resolution
    Extent measure(void* display, const RenderTable& rt) const;

    // Legacy: measure using GC's current font (single-font only)
    Extent measureWithGC(void* display, void* gc) const;

    // ── Drawing ──────────────────────────────────────────────────
    // Draw using a RenderTable for multi-font support
    void draw(void* display, unsigned long window, void* gc,
              const RenderTable& rt, int x, int y) const;

    // Legacy: draw using GC's current font
    void drawWithGC(void* display, unsigned long window, void* gc,
                    int x, int y) const;

    // ── Serialization (replaces XmStringGetLtoR / XmCvtXmStringToText) ─
    std::vector<uint8_t> serialize() const;
    static CompoundString deserialize(const uint8_t* data, size_t size);

private:
    std::vector<StringSegment> segments_;
};

// ── RenderTable ──────────────────────────────────────────────────────
// Replaces XmRenderTable / XmFontList.
// Maps font tags to X11 font info for rendering CompoundStrings.

class RenderTable {
public:
    RenderTable() = default;
    RenderTable(const RenderTable&) = default;
    RenderTable& operator=(const RenderTable&) = default;
    RenderTable(RenderTable&&) = default;
    RenderTable& operator=(RenderTable&&) = default;

    struct Rendition {
        std::string tag;          // matches StringSegment::fontTag
        std::string fontName;     // XLFD, Xft name, or "default"
        unsigned long foreground = 0;
        unsigned long background = ~0UL;
        int tabSize = 8;
        // X11 font struct cached at realize time (legacy path)
        void* fontStruct = nullptr;
        // FontRenderer handle (modern path, used when available)
        std::shared_ptr<motif::core::Font> fontHandle;
    };

    void addRendition(const Rendition& r);
    void removeRendition(const std::string& tag);
    const Rendition* findRendition(const std::string& tag) const;

    bool empty() const { return renditions_.empty(); }
    size_t size() const { return renditions_.size(); }

    void realize(void* display);
    void free(void* display);

    // Create a default table with the default X11 font, realized for the given Display
    static RenderTable defaultTable(void* display);

private:
    std::vector<Rendition> renditions_;
};

} // namespace motif
