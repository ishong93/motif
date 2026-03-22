#include <motif/core/Shell.h>
#include <motif/core/Application.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

namespace motif {

Shell::Shell(Widget* parent) : Widget(parent) {
    resources_.set<std::string>("title", "");
    resources_.set<std::string>("iconName", "");
    resources_.set<int>("minWidth", 0);
    resources_.set<int>("minHeight", 0);
    resources_.set<int>("maxWidth", 0);
    resources_.set<int>("maxHeight", 0);
}

Shell::~Shell() = default;

std::vector<ResourceSpec> Shell::resourceSpecs() const {
    auto specs = Widget::resourceSpecs();
    specs.emplace_back("title", "Title", typeid(std::string), 0,
                       ResourceValue{std::string("")});
    specs.emplace_back("iconName", "IconName", typeid(std::string), 0,
                       ResourceValue{std::string("")});
    specs.emplace_back("minWidth", "MinWidth", typeid(int), 0, ResourceValue{0});
    specs.emplace_back("minHeight", "MinHeight", typeid(int), 0, ResourceValue{0});
    specs.emplace_back("maxWidth", "MaxWidth", typeid(int), 0, ResourceValue{0});
    specs.emplace_back("maxHeight", "MaxHeight", typeid(int), 0, ResourceValue{0});
    return specs;
}

void Shell::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "title") {
        setTitle(resources_.get<std::string>("title", ""));
        return;
    } else if (resourceName == "iconName") {
        iconName_ = resources_.get<std::string>("iconName", "");
    }
    Widget::onResourceChanged(resourceName);
}

void Shell::createWindow() {
    auto& app = Application::instance();
    auto* dpy = app.display();
    if (!dpy) return;

    int w = (width_ > 0) ? width_ : 400;
    int h = (height_ > 0) ? height_ : 300;
    width_ = w;
    height_ = h;

    long eventMask = ExposureMask | StructureNotifyMask
                   | ButtonPressMask | ButtonReleaseMask
                   | KeyPressMask | KeyReleaseMask
                   | FocusChangeMask;

    XSetWindowAttributes attrs;
    attrs.event_mask = eventMask;
    attrs.background_pixel = app.whitePixel();

    window_ = XCreateWindow(
        dpy, app.rootWindow(),
        x_, y_, w, h,
        0,
        CopyFromParent,
        InputOutput,
        CopyFromParent,
        CWEventMask | CWBackPixel,
        &attrs
    );

    gc_ = XCreateGC(dpy, window_, 0, nullptr);
    app.registerWidget(window_, this);

    // Set WM_DELETE_WINDOW protocol
    Atom wmDelete = app.wmDeleteWindow();
    XSetWMProtocols(dpy, window_, &wmDelete, 1);

    // Set title
    if (!title_.empty()) {
        XStoreName(dpy, window_, title_.c_str());
    }

    // Set icon name
    if (!iconName_.empty()) {
        XSetIconName(dpy, window_, iconName_.c_str());
    }

    // Set size hints
    XSizeHints hints;
    hints.flags = 0;

    if (minWidth_ > 0 && minHeight_ > 0) {
        hints.flags |= PMinSize;
        hints.min_width = minWidth_;
        hints.min_height = minHeight_;
    }
    if (maxWidth_ > 0 && maxHeight_ > 0) {
        hints.flags |= PMaxSize;
        hints.max_width = maxWidth_;
        hints.max_height = maxHeight_;
    }

    if (hints.flags) {
        XSetWMNormalHints(dpy, window_, &hints);
    }
}

void Shell::realize() {
    if (realized_) return;

    createWindow();
    realized_ = true;

    // Realize children
    for (auto* child : children_) {
        child->realize();
    }

    // Map the shell window
    if (visible_ && window_) {
        auto* dpy = Application::instance().display();
        XMapWindow(dpy, window_);
    }
}

void Shell::setTitle(const std::string& title) {
    title_ = title;
    if (window_) {
        XStoreName(Application::instance().display(), window_, title_.c_str());
    }
}

void Shell::handleClose() {
    if (closeCb_) {
        closeCb_();
    } else {
        Application::instance().quit();
    }
}

void Shell::handleConfigure(int x, int y, int w, int h) {
    Widget::handleConfigure(x, y, w, h);

    // Relay size to the single managed child (if any)
    if (!children_.empty()) {
        auto* child = children_.front();
        child->setPosition(0, 0, w, h);
    }
}

} // namespace motif
