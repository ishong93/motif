#include <motif/widgets/CascadeButton.h>
#include <motif/menus/PopupMenu.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>

namespace motif {

CascadeButton::CascadeButton(Widget* parent) : Label(parent) {}
CascadeButton::~CascadeButton() = default;

std::vector<ResourceSpec> CascadeButton::resourceSpecs() const {
    auto specs = Label::resourceSpecs();
    // subMenu is a pointer, not stored as resource value — set programmatically
    return specs;
}

void CascadeButton::expose() {
    Label::expose();
    if (!window_ || !gc_) return;

    // Draw cascade indicator (small arrow on right)
    auto* dpy = Application::instance().display();
    int arrowSz = 6;
    int ax = width_ - arrowSz - 4;
    int ay = height_ / 2;

    XPoint pts[3] = {
        {(short)ax, (short)(ay - arrowSz)},
        {(short)(ax + arrowSz), (short)ay},
        {(short)ax, (short)(ay + arrowSz)}
    };
    XSetForeground(dpy, gc_, foreground_);
    XFillPolygon(dpy, window_, gc_, pts, 3, Convex, CoordModeOrigin);
}

void CascadeButton::handleButtonPress(unsigned int button, int x, int y) {
    if (button == 1 && sensitive_) {
        invokeCallbacks(CascadingCallback);
    }
}

void CascadeButton::handleButtonRelease(unsigned int button, int x, int y) {
    if (button == 1 && sensitive_ && x >= 0 && x < width_ && y >= 0 && y < height_) {
        invokeCallbacks(ActivateCallback);
    }
}

} // namespace motif
