#pragma once

#include <motif/core/Widget.h>

namespace motif {

// Gadget: lightweight widget that shares its parent's X window.
// Renders into the parent's window instead of having its own.
class Gadget : public Widget {
public:
    Gadget() = default;
    explicit Gadget(Widget* parent);
    ~Gadget() override;

    void realize() override;
    void show() override;
    void hide() override;
    void move(int x, int y) override;
    void resize(int width, int height) override;

    bool isGadget() const { return true; }

    // Hit testing: does (px, py) in parent coordinates fall within this gadget?
    bool containsPoint(int px, int py) const;

    void setHighlightThickness(int t) { highlightThickness_ = t; }
    int highlightThickness() const { return highlightThickness_; }

    void setShadowThickness(int t) { shadowThickness_ = t; }
    int shadowThickness() const { return shadowThickness_; }

    void setForeground(unsigned long pixel) { foreground_ = pixel; }
    unsigned long foreground() const { return foreground_; }

    // Resource system
    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

protected:
    void expose() override;
    void drawHighlight();
    void eraseHighlight();
    void drawShadow(bool raised);

    // Helpers: parent's window/GC (gadgets have no own window)
    XWindow parentWindow() const;
    XGC parentGC() const;

    int highlightThickness_ = 2;
    int shadowThickness_ = 2;
    unsigned long foreground_ = 0;
    unsigned long background_ = ~0UL;
    bool focused_ = false;
};

} // namespace motif
