#pragma once

#include <motif/core/Widget.h>

namespace motif {

class Primitive : public Widget {
public:
    Primitive() = default;
    explicit Primitive(Widget* parent);
    ~Primitive() override;

    // Focus & traversal
    virtual bool acceptsFocus() const;
    bool hasFocus() const { return focused_; }

    // Appearance
    void setHighlightThickness(int thickness) { highlightThickness_ = thickness; }
    int highlightThickness() const { return highlightThickness_; }

    void setShadowThickness(int thickness) { shadowThickness_ = thickness; }
    int shadowThickness() const { return shadowThickness_; }

    void setForeground(unsigned long pixel) { foreground_ = pixel; }
    unsigned long foreground() const { return foreground_; }

    void setBackground(unsigned long pixel);
    unsigned long background() const { return background_; }

    // Resource system
    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    // Event handlers
    void handleFocusIn() override;
    void handleFocusOut() override;

protected:
    void drawHighlight();
    void eraseHighlight();
    void drawShadow(bool raised);

    bool focused_ = false;
    int highlightThickness_ = 2;
    int shadowThickness_ = 2;
    unsigned long foreground_ = 0;   // black
    unsigned long background_ = ~0UL; // white (set properly on realize)
};

} // namespace motif
