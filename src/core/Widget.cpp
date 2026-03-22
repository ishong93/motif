#include <motif/core/Widget.h>
#include <motif/core/Application.h>
#include <motif/core/TypeRegistry.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <algorithm>
#include <sstream>

namespace motif {

Widget::Widget(Widget* parent) : parent_(parent) {
    if (parent_) {
        parent_->addChild(this);
    }
    // Ensure TypeRegistry builtins are available
    TypeRegistry::instance().registerBuiltins();
}

Widget::~Widget() {
    destroy();
}

Widget::Widget(Widget&& other) noexcept
    : parent_(other.parent_)
    , children_(std::move(other.children_))
    , window_(other.window_)
    , gc_(other.gc_)
    , x_(other.x_), y_(other.y_)
    , width_(other.width_), height_(other.height_)
    , realized_(other.realized_)
    , visible_(other.visible_)
    , sensitive_(other.sensitive_)
    , name_(std::move(other.name_))
    , callbacks_(std::move(other.callbacks_)) {
    other.parent_ = nullptr;
    other.window_ = 0;
    other.gc_ = nullptr;
    other.realized_ = false;

    // Re-register in Application widget map
    if (window_) {
        Application::instance().registerWidget(window_, this);
    }
    for (auto* child : children_) {
        child->parent_ = this;
    }
}

Widget& Widget::operator=(Widget&& other) noexcept {
    if (this != &other) {
        destroy();
        parent_ = other.parent_;
        children_ = std::move(other.children_);
        window_ = other.window_;
        gc_ = other.gc_;
        x_ = other.x_; y_ = other.y_;
        width_ = other.width_; height_ = other.height_;
        realized_ = other.realized_;
        visible_ = other.visible_;
        sensitive_ = other.sensitive_;
        name_ = std::move(other.name_);
        callbacks_ = std::move(other.callbacks_);
        other.parent_ = nullptr;
        other.window_ = 0;
        other.gc_ = nullptr;
        other.realized_ = false;

        if (window_) {
            Application::instance().registerWidget(window_, this);
        }
        for (auto* child : children_) {
            child->parent_ = this;
        }
    }
    return *this;
}

// ── Lifecycle ───────────────────────────────────────────────

void Widget::realize() {
    if (realized_) return;

    createWindow();
    realized_ = true;

    for (auto* child : children_) {
        child->realize();
    }

    if (visible_ && window_) {
        auto* dpy = Application::instance().display();
        XMapWindow(dpy, window_);
    }
}

void Widget::destroy() {
    if (parent_) {
        parent_->removeChild(this);
        parent_ = nullptr;
    }

    auto childrenCopy = children_;
    for (auto* child : childrenCopy) {
        child->parent_ = nullptr;
        child->destroy();
    }
    children_.clear();

    destroyWindow();
    realized_ = false;
}

void Widget::createWindow() {
    auto& app = Application::instance();
    auto* dpy = app.display();
    if (!dpy) return;

    // Determine parent X window
    XWindow parentWin = (parent_ && parent_->window_)
        ? parent_->window_
        : app.rootWindow();

    // Ensure minimum size
    int w = (width_ > 0) ? width_ : 1;
    int h = (height_ > 0) ? height_ : 1;

    // Select events
    long eventMask = ExposureMask | StructureNotifyMask
                   | ButtonPressMask | ButtonReleaseMask
                   | PointerMotionMask | ButtonMotionMask
                   | EnterWindowMask | LeaveWindowMask
                   | KeyPressMask | KeyReleaseMask
                   | FocusChangeMask;

    XSetWindowAttributes attrs;
    attrs.event_mask = eventMask;
    attrs.background_pixel = app.whitePixel();

    window_ = XCreateWindow(
        dpy, parentWin,
        x_, y_, w, h,
        0,                          // border width
        CopyFromParent,             // depth
        InputOutput,                // class
        CopyFromParent,             // visual
        CWEventMask | CWBackPixel,  // value mask
        &attrs
    );

    // Create GC for this window
    gc_ = XCreateGC(dpy, window_, 0, nullptr);

    // Register for event dispatch
    app.registerWidget(window_, this);
}

void Widget::destroyWindow() {
    if (!window_) return;

    auto& app = Application::instance();
    auto* dpy = app.display();

    app.unregisterWidget(window_);

    if (gc_ && dpy) {
        XFreeGC(dpy, gc_);
        gc_ = nullptr;
    }
    if (dpy) {
        XDestroyWindow(dpy, window_);
    }
    window_ = 0;
}

// ── Geometry ────────────────────────────────────────────────

void Widget::resize(int width, int height) {
    width_ = width;
    height_ = height;
    if (window_) {
        auto* dpy = Application::instance().display();
        XResizeWindow(dpy, window_, width_, height_);
    }
}

void Widget::move(int x, int y) {
    x_ = x;
    y_ = y;
    if (window_) {
        auto* dpy = Application::instance().display();
        XMoveWindow(dpy, window_, x_, y_);
    }
}

void Widget::setPosition(int x, int y, int width, int height) {
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
    if (window_) {
        auto* dpy = Application::instance().display();
        XMoveResizeWindow(dpy, window_, x_, y_, width_, height_);
    }
}

// ── Hierarchy ───────────────────────────────────────────────

void Widget::addChild(Widget* child) {
    children_.push_back(child);
}

void Widget::removeChild(Widget* child) {
    children_.erase(
        std::remove(children_.begin(), children_.end(), child),
        children_.end()
    );
}

// ── Visibility ──────────────────────────────────────────────

void Widget::show() {
    visible_ = true;
    if (window_) {
        XMapWindow(Application::instance().display(), window_);
    }
}

void Widget::hide() {
    visible_ = false;
    if (window_) {
        XUnmapWindow(Application::instance().display(), window_);
    }
}

void Widget::setSensitive(bool sensitive) {
    sensitive_ = sensitive;
}

// ── Callbacks ───────────────────────────────────────────────

void Widget::addCallback(const std::string& reason, Callback cb) {
    callbacks_.push_back({reason, std::move(cb)});
}

void Widget::removeCallbacks(const std::string& reason) {
    callbacks_.erase(
        std::remove_if(callbacks_.begin(), callbacks_.end(),
            [&reason](const CallbackEntry& e) { return e.reason == reason; }),
        callbacks_.end()
    );
}

void Widget::invokeCallbacks(const std::string& reason, void* callData) {
    for (auto& entry : callbacks_) {
        if (entry.reason == reason) {
            entry.callback(*this, callData);
        }
    }
}

// ── Event Handlers ──────────────────────────────────────────

void Widget::handleExpose() {
    expose();
}

void Widget::handleConfigure(int x, int y, int w, int h) {
    x_ = x;
    y_ = y;
    width_ = w;
    height_ = h;
}

void Widget::handleMap() {
    visible_ = true;
}

void Widget::handleUnmap() {
    visible_ = false;
}

void Widget::handleButtonPress(unsigned int /*button*/, int /*x*/, int /*y*/) {
    // Override in subclasses (e.g., PushButton::arm)
}

void Widget::handleButtonRelease(unsigned int /*button*/, int /*x*/, int /*y*/) {
    // Override in subclasses
}

void Widget::handleMotionNotify(int /*x*/, int /*y*/, unsigned int /*state*/) {
    // Override in subclasses (e.g., Scale/ScrollBar drag)
}

void Widget::handleEnter(int /*x*/, int /*y*/) {}
void Widget::handleLeave() {}

void Widget::handleKeyPress(unsigned int /*keycode*/, unsigned int /*state*/) {}
void Widget::handleKeyRelease(unsigned int /*keycode*/, unsigned int /*state*/) {}

void Widget::handleFocusIn() {}
void Widget::handleFocusOut() {}

void Widget::handleDestroy() {
    window_ = 0;
    gc_ = nullptr;
    realized_ = false;
}

void Widget::handleClose() {
    // Default: quit the application
    Application::instance().quit();
}

// ── Manage ──────────────────────────────────────────────────

void Widget::manage() {
    managed_ = true;
    if (parent_ && parent_->isRealized() && !realized_) {
        realize();
    }
    show();
}

void Widget::unmanage() {
    managed_ = false;
    hide();
}

// ── Resource integration ────────────────────────────────────

void Widget::onResourceChanged(const std::string& /*resourceName*/) {
    // Base: re-expose if realized
    if (realized_) {
        handleExpose();
    }
}

std::string Widget::widgetPath() const {
    if (!parent_) return name_;
    std::string parentPath = parent_->widgetPath();
    if (parentPath.empty()) return name_;
    return parentPath + "." + name_;
}

void Widget::initializeResources() {
    auto specs = resourceSpecs();
    if (!specs.empty()) {
        resources_.applyDefaults(widgetPath(), specs);
    }
}

// ── Drawing ─────────────────────────────────────────────────

void Widget::expose() {
    // Base implementation: clear window
    if (window_) {
        auto* dpy = Application::instance().display();
        XClearWindow(dpy, window_);
    }
}

} // namespace motif
