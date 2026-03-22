#pragma once

#include <motif/core/Widget.h>

namespace motif {

class Manager : public Widget {
public:
    Manager() = default;
    explicit Manager(Widget* parent);
    ~Manager() override;

    // Layout
    virtual void layout();
    virtual void childChanged(Widget* child);

    // Appearance
    void setShadowThickness(int thickness) { shadowThickness_ = thickness; }
    int shadowThickness() const { return shadowThickness_; }

    void setForeground(unsigned long pixel) { foreground_ = pixel; }
    unsigned long foreground() const { return foreground_; }
    void setBackground(unsigned long pixel);
    unsigned long background() const { return background_; }

    // Focus traversal among children
    virtual Widget* nextFocusChild(Widget* current);
    virtual Widget* prevFocusChild(Widget* current);

    // Resource system
    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    // Event handlers
    void handleConfigure(int x, int y, int w, int h) override;
    void handleExpose() override;
    void handleButtonPress(unsigned int button, int x, int y) override;
    void handleButtonRelease(unsigned int button, int x, int y) override;
    void handleMotionNotify(int x, int y, unsigned int state) override;
    void handleEnter(int x, int y) override;
    void handleLeave() override;

protected:
    void expose() override;
    void drawShadow();

    // Find gadget child at (x,y) in this manager's coordinate space
    Widget* gadgetAt(int x, int y) const;

    int shadowThickness_ = 0;
    unsigned long foreground_ = 0;
    unsigned long background_ = ~0UL;
};

} // namespace motif
