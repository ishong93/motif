#pragma once

#include <motif/core/Widget.h>
#include <string>
#include <vector>
#include <cstdint>

namespace motif {

// A small override-redirect window that follows the cursor during drag.
// Supports multiple visual states (idle, accept, reject) and icon rendering.
class DragIcon {
public:
    /// Visual state shown during drag
    enum class State {
        Idle,       // default drag appearance
        Accept,     // target accepts drop (green indicator)
        Reject      // target rejects drop (red X indicator)
    };

    /// Icon type for the drag cursor
    enum class IconType {
        Text,       // text label only
        Copy,       // copy indicator (+)
        Move,       // move indicator (arrow)
        Link,       // link indicator (chain)
        Custom      // custom pixmap
    };

    DragIcon();
    ~DragIcon();

    void setText(const std::string& text) { text_ = text; }
    void setSize(int w, int h) { width_ = w; height_ = h; }
    void setIconType(IconType type) { iconType_ = type; }

    /// Set the visual state (updates appearance immediately if visible)
    void setState(State state);
    State state() const { return state_; }

    /// Set a custom pixmap icon (XPM data)
    void setPixmapData(const std::vector<uint8_t>& data, int w, int h);

    void show(int rootX, int rootY);
    void hide();
    void moveTo(int rootX, int rootY);
    bool isVisible() const { return visible_; }

private:
    void create();
    void destroy();
    void redraw();
    void drawStateIndicator();

    XWindow window_ = 0;
    XGC gc_ = nullptr;
    std::string text_;
    int width_ = 80;
    int height_ = 28;
    bool visible_ = false;
    State state_ = State::Idle;
    IconType iconType_ = IconType::Text;
};

} // namespace motif
