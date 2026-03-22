#include <motif/wm/Decoration.h>
#include <motif/wm/WindowManager.h>

#include <X11/Xutil.h>

namespace motif::wm {

Decoration::Decoration(WindowManager& wm) : wm_(wm) {}

Decoration::~Decoration() {
    if (gc_) {
        XFreeGC(wm_.display(), gc_);
        gc_ = nullptr;
    }
    if (font_) {
        XFreeFont(wm_.display(), font_);
        font_ = nullptr;
    }
}

void Decoration::loadFont() {
    font_ = XLoadQueryFont(wm_.display(), wm_.config().fontName.c_str());
    if (!font_) {
        font_ = XLoadQueryFont(wm_.display(), "fixed");
    }
}

void Decoration::createFrame(ClientWindow* client) {
    auto* dpy = wm_.display();
    auto& cfg = wm_.config();

    int frameW = client->width + 2 * cfg.borderWidth;
    int frameH = client->height + cfg.titleBarHeight + 2 * cfg.borderWidth;

    // Frame window
    XSetWindowAttributes attrs;
    attrs.background_pixel = cfg.borderColor;
    attrs.border_pixel = BlackPixel(dpy, wm_.screen());
    attrs.event_mask = SubstructureRedirectMask | SubstructureNotifyMask |
                       ExposureMask | ButtonPressMask | ButtonReleaseMask |
                       EnterWindowMask;

    client->frame = XCreateWindow(
        dpy, wm_.root(),
        client->x, client->y, frameW, frameH,
        0, CopyFromParent, InputOutput, CopyFromParent,
        CWBackPixel | CWBorderPixel | CWEventMask, &attrs
    );

    // Title bar window
    attrs.background_pixel = cfg.unfocusedTitleBg;
    attrs.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask |
                       ButtonMotionMask;

    client->titleBar = XCreateWindow(
        dpy, client->frame,
        cfg.borderWidth, cfg.borderWidth,
        frameW - 2 * cfg.borderWidth, cfg.titleBarHeight,
        0, CopyFromParent, InputOutput, CopyFromParent,
        CWBackPixel | CWEventMask, &attrs
    );

    // Close button (leftmost)
    int btnY = (cfg.titleBarHeight - cfg.buttonSize) / 2;

    attrs.background_pixel = cfg.unfocusedTitleBg;
    attrs.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask;

    client->closeBtn = XCreateWindow(
        dpy, client->titleBar,
        4, btnY, cfg.buttonSize, cfg.buttonSize,
        0, CopyFromParent, InputOutput, CopyFromParent,
        CWBackPixel | CWEventMask, &attrs
    );

    // Maximize button (right side)
    int titleW = frameW - 2 * cfg.borderWidth;
    client->maximizeBtn = XCreateWindow(
        dpy, client->titleBar,
        titleW - cfg.buttonSize - 4 - cfg.buttonSize - 4, btnY,
        cfg.buttonSize, cfg.buttonSize,
        0, CopyFromParent, InputOutput, CopyFromParent,
        CWBackPixel | CWEventMask, &attrs
    );

    // Minimize button
    client->minimizeBtn = XCreateWindow(
        dpy, client->titleBar,
        titleW - cfg.buttonSize - 4, btnY,
        cfg.buttonSize, cfg.buttonSize,
        0, CopyFromParent, InputOutput, CopyFromParent,
        CWBackPixel | CWEventMask, &attrs
    );

    // Create GC for drawing
    if (!gc_) {
        gc_ = XCreateGC(dpy, client->frame, 0, nullptr);
        if (font_) {
            XSetFont(dpy, gc_, font_->fid);
        }
    }

    // Map everything
    XMapWindow(dpy, client->closeBtn);
    XMapWindow(dpy, client->maximizeBtn);
    XMapWindow(dpy, client->minimizeBtn);
    XMapWindow(dpy, client->titleBar);
    XMapWindow(dpy, client->frame);
}

void Decoration::destroyFrame(ClientWindow* client) {
    auto* dpy = wm_.display();

    // Reparent client back to root before destroying frame
    XReparentWindow(dpy, client->client, wm_.root(), client->x, client->y);
    XRemoveFromSaveSet(dpy, client->client);

    XDestroyWindow(dpy, client->frame);
    client->frame = None;
    client->titleBar = None;
    client->closeBtn = None;
    client->maximizeBtn = None;
    client->minimizeBtn = None;
}

void Decoration::drawTitleBar(ClientWindow* client) {
    if (client->titleBar == None || !gc_) return;
    auto* dpy = wm_.display();
    auto& cfg = wm_.config();

    unsigned long bg = client->focused ? cfg.focusedTitleBg : cfg.unfocusedTitleBg;
    unsigned long fg = client->focused ? cfg.focusedTitleFg : cfg.unfocusedTitleFg;

    XSetWindowBackground(dpy, client->titleBar, bg);
    XClearWindow(dpy, client->titleBar);

    // Draw title text
    if (font_ && !client->title.empty()) {
        int textW = XTextWidth(font_, client->title.c_str(),
                               static_cast<int>(client->title.size()));
        int titleW = client->width; // approximate
        int tx = cfg.buttonSize + 12; // after close button
        int ty = (cfg.titleBarHeight + font_->ascent - font_->descent) / 2;

        XSetForeground(dpy, gc_, fg);
        XDrawString(dpy, client->titleBar, gc_, tx, ty,
                    client->title.c_str(), static_cast<int>(client->title.size()));
    }

    // Redraw buttons
    drawCloseButton(client);
    drawMaximizeButton(client);
    drawMinimizeButton(client);

    // 3D border at bottom of title bar
    XSetForeground(dpy, gc_, BlackPixel(dpy, wm_.screen()));
    XDrawLine(dpy, client->titleBar, gc_, 0, cfg.titleBarHeight - 1,
              client->width + 2 * cfg.borderWidth, cfg.titleBarHeight - 1);
}

void Decoration::drawBorder(ClientWindow* client) {
    if (client->frame == None || !gc_) return;
    auto* dpy = wm_.display();
    auto& cfg = wm_.config();

    int frameW = client->width + 2 * cfg.borderWidth;
    int frameH = client->height + cfg.titleBarHeight + 2 * cfg.borderWidth;

    // 3D beveled frame
    unsigned long light = WhitePixel(dpy, wm_.screen());
    unsigned long dark = BlackPixel(dpy, wm_.screen());

    // Outer highlight (top-left)
    XSetForeground(dpy, gc_, light);
    XDrawLine(dpy, client->frame, gc_, 0, 0, frameW - 1, 0);
    XDrawLine(dpy, client->frame, gc_, 0, 0, 0, frameH - 1);

    // Outer shadow (bottom-right)
    XSetForeground(dpy, gc_, dark);
    XDrawLine(dpy, client->frame, gc_, 0, frameH - 1, frameW - 1, frameH - 1);
    XDrawLine(dpy, client->frame, gc_, frameW - 1, 0, frameW - 1, frameH - 1);
}

void Decoration::drawCloseButton(ClientWindow* client, bool pressed) {
    if (client->closeBtn == None || !gc_) return;
    auto* dpy = wm_.display();
    auto& cfg = wm_.config();

    unsigned long bg = client->focused ? cfg.focusedTitleBg : cfg.unfocusedTitleBg;
    unsigned long fg = client->focused ? cfg.focusedTitleFg : cfg.unfocusedTitleFg;

    XSetWindowBackground(dpy, client->closeBtn, bg);
    XClearWindow(dpy, client->closeBtn);

    int s = cfg.buttonSize;
    int p = 4; // padding

    // Draw X
    XSetForeground(dpy, gc_, fg);
    XSetLineAttributes(dpy, gc_, 2, LineSolid, CapButt, JoinMiter);
    XDrawLine(dpy, client->closeBtn, gc_, p, p, s - p, s - p);
    XDrawLine(dpy, client->closeBtn, gc_, s - p, p, p, s - p);
    XSetLineAttributes(dpy, gc_, 1, LineSolid, CapButt, JoinMiter);

    // 3D border
    unsigned long light = pressed ? BlackPixel(dpy, wm_.screen()) : WhitePixel(dpy, wm_.screen());
    unsigned long dark = pressed ? WhitePixel(dpy, wm_.screen()) : BlackPixel(dpy, wm_.screen());
    XSetForeground(dpy, gc_, light);
    XDrawLine(dpy, client->closeBtn, gc_, 0, 0, s - 1, 0);
    XDrawLine(dpy, client->closeBtn, gc_, 0, 0, 0, s - 1);
    XSetForeground(dpy, gc_, dark);
    XDrawLine(dpy, client->closeBtn, gc_, 0, s - 1, s - 1, s - 1);
    XDrawLine(dpy, client->closeBtn, gc_, s - 1, 0, s - 1, s - 1);
}

void Decoration::drawMaximizeButton(ClientWindow* client, bool pressed) {
    if (client->maximizeBtn == None || !gc_) return;
    auto* dpy = wm_.display();
    auto& cfg = wm_.config();

    unsigned long bg = client->focused ? cfg.focusedTitleBg : cfg.unfocusedTitleBg;
    unsigned long fg = client->focused ? cfg.focusedTitleFg : cfg.unfocusedTitleFg;

    XSetWindowBackground(dpy, client->maximizeBtn, bg);
    XClearWindow(dpy, client->maximizeBtn);

    int s = cfg.buttonSize;
    int p = 3;

    // Draw box icon
    XSetForeground(dpy, gc_, fg);
    XDrawRectangle(dpy, client->maximizeBtn, gc_, p, p, s - 2 * p, s - 2 * p);
    XDrawLine(dpy, client->maximizeBtn, gc_, p, p + 1, s - p, p + 1);

    // 3D border
    unsigned long light = pressed ? BlackPixel(dpy, wm_.screen()) : WhitePixel(dpy, wm_.screen());
    unsigned long dark = pressed ? WhitePixel(dpy, wm_.screen()) : BlackPixel(dpy, wm_.screen());
    XSetForeground(dpy, gc_, light);
    XDrawLine(dpy, client->maximizeBtn, gc_, 0, 0, s - 1, 0);
    XDrawLine(dpy, client->maximizeBtn, gc_, 0, 0, 0, s - 1);
    XSetForeground(dpy, gc_, dark);
    XDrawLine(dpy, client->maximizeBtn, gc_, 0, s - 1, s - 1, s - 1);
    XDrawLine(dpy, client->maximizeBtn, gc_, s - 1, 0, s - 1, s - 1);
}

void Decoration::drawMinimizeButton(ClientWindow* client, bool pressed) {
    if (client->minimizeBtn == None || !gc_) return;
    auto* dpy = wm_.display();
    auto& cfg = wm_.config();

    unsigned long bg = client->focused ? cfg.focusedTitleBg : cfg.unfocusedTitleBg;
    unsigned long fg = client->focused ? cfg.focusedTitleFg : cfg.unfocusedTitleFg;

    XSetWindowBackground(dpy, client->minimizeBtn, bg);
    XClearWindow(dpy, client->minimizeBtn);

    int s = cfg.buttonSize;

    // Draw horizontal line (minimize icon)
    XSetForeground(dpy, gc_, fg);
    int lineY = s - 5;
    XSetLineAttributes(dpy, gc_, 2, LineSolid, CapButt, JoinMiter);
    XDrawLine(dpy, client->minimizeBtn, gc_, 4, lineY, s - 4, lineY);
    XSetLineAttributes(dpy, gc_, 1, LineSolid, CapButt, JoinMiter);

    // 3D border
    unsigned long light = pressed ? BlackPixel(dpy, wm_.screen()) : WhitePixel(dpy, wm_.screen());
    unsigned long dark = pressed ? WhitePixel(dpy, wm_.screen()) : BlackPixel(dpy, wm_.screen());
    XSetForeground(dpy, gc_, light);
    XDrawLine(dpy, client->minimizeBtn, gc_, 0, 0, s - 1, 0);
    XDrawLine(dpy, client->minimizeBtn, gc_, 0, 0, 0, s - 1);
    XSetForeground(dpy, gc_, dark);
    XDrawLine(dpy, client->minimizeBtn, gc_, 0, s - 1, s - 1, s - 1);
    XDrawLine(dpy, client->minimizeBtn, gc_, s - 1, 0, s - 1, s - 1);
}

void Decoration::updateFocusDecoration(ClientWindow* client) {
    drawTitleBar(client);
    drawBorder(client);
}

} // namespace motif::wm
