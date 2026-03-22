#include <motif/graphics/Draw.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>

namespace motif {
namespace draw {

void drawShadow(Widget* w, int x, int y, int width, int height,
                int thickness, bool raised) {
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();
    auto gc = w->gc();
    auto win = w->window();
    if (!dpy || !win || !gc) return;

    unsigned long top = raised ? app.whitePixel() : app.blackPixel();
    unsigned long bot = raised ? app.blackPixel() : app.whitePixel();

    for (int i = 0; i < thickness; ++i) {
        XSetForeground(dpy, gc, top);
        XDrawLine(dpy, win, gc, x + i, y + i, x + width - 1 - i, y + i);
        XDrawLine(dpy, win, gc, x + i, y + i, x + i, y + height - 1 - i);
        XSetForeground(dpy, gc, bot);
        XDrawLine(dpy, win, gc, x + width - 1 - i, y + i, x + width - 1 - i, y + height - 1 - i);
        XDrawLine(dpy, win, gc, x + i, y + height - 1 - i, x + width - 1 - i, y + height - 1 - i);
    }
}

void drawEtchedShadow(Widget* w, int x, int y, int width, int height,
                       int thickness, bool raised) {
    int half = thickness / 2;
    drawShadow(w, x, y, width, height, half, raised);
    drawShadow(w, x + half, y + half, width - 2 * half, height - 2 * half, half, !raised);
}

void drawArrow(Widget* w, int x, int y, int size, int direction, bool raised) {
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();
    auto gc = w->gc();
    auto win = w->window();
    if (!dpy || !win || !gc) return;

    XPoint pts[3];
    switch (direction) {
        case 0: // up
            pts[0] = {(short)(x + size/2), (short)y};
            pts[1] = {(short)x, (short)(y + size)};
            pts[2] = {(short)(x + size), (short)(y + size)};
            break;
        case 1: // down
            pts[0] = {(short)(x + size/2), (short)(y + size)};
            pts[1] = {(short)x, (short)y};
            pts[2] = {(short)(x + size), (short)y};
            break;
        case 2: // left
            pts[0] = {(short)x, (short)(y + size/2)};
            pts[1] = {(short)(x + size), (short)y};
            pts[2] = {(short)(x + size), (short)(y + size)};
            break;
        case 3: // right
            pts[0] = {(short)(x + size), (short)(y + size/2)};
            pts[1] = {(short)x, (short)y};
            pts[2] = {(short)x, (short)(y + size)};
            break;
    }

    XSetForeground(dpy, gc, raised ? app.whitePixel() : app.blackPixel());
    XFillPolygon(dpy, win, gc, pts, 3, Convex, CoordModeOrigin);
    XSetForeground(dpy, gc, app.blackPixel());
    XDrawLines(dpy, win, gc, pts, 3, CoordModeOrigin);
}

void drawHighlight(Widget* w, int x, int y, int width, int height, int thickness) {
    auto* dpy = Application::instance().display();
    auto gc = w->gc();
    auto win = w->window();
    if (!dpy || !win || !gc) return;

    XSetForeground(dpy, gc, Application::instance().blackPixel());
    for (int i = 0; i < thickness; ++i) {
        XDrawRectangle(dpy, win, gc, x + i, y + i, width - 2*i - 1, height - 2*i - 1);
    }
}

void drawSeparator(Widget* w, int x1, int y1, int x2, int y2, int type) {
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();
    auto gc = w->gc();
    auto win = w->window();
    if (!dpy || !win || !gc) return;

    XSetForeground(dpy, gc, app.blackPixel());
    XDrawLine(dpy, win, gc, x1, y1, x2, y2);
}

} // namespace draw
} // namespace motif
