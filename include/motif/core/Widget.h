#pragma once

#include <motif/core/Resource.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

using XWindow = unsigned long;
using XGC = struct _XGC*;

namespace motif {

class Widget {
public:
    using Callback = std::function<void(Widget&, void*)>;

    Widget() = default;
    explicit Widget(Widget* parent);
    virtual ~Widget();

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget(Widget&&) noexcept;
    Widget& operator=(Widget&&) noexcept;

    // Lifecycle
    virtual void realize();
    virtual void destroy();
    bool isRealized() const { return realized_; }
    void manage();
    void unmanage();
    bool isManaged() const { return managed_; }

    // Geometry
    virtual void resize(int width, int height);
    virtual void move(int x, int y);
    void setPosition(int x, int y, int width, int height);

    int x() const { return x_; }
    int y() const { return y_; }
    int width() const { return width_; }
    int height() const { return height_; }

    // Hierarchy
    Widget* parent() const { return parent_; }
    const std::vector<Widget*>& children() const { return children_; }
    void addChild(Widget* child);
    void removeChild(Widget* child);

    // Visibility
    virtual void show();
    virtual void hide();
    bool isVisible() const { return visible_; }

    // Sensitivity (enabled/disabled)
    virtual void setSensitive(bool sensitive);
    bool isSensitive() const { return sensitive_; }

    // Type query (gadgets share parent window)
    virtual bool isGadget() const { return false; }

    // Name
    void setName(const std::string& name) { name_ = name; }
    const std::string& name() const { return name_; }

    // X11 window
    XWindow window() const { return window_; }
    XGC gc() const { return gc_; }

    // ── Resource system integration ─────────────────────────────
    // Provides a type-safe, Xrm-compatible resource mechanism
    // replacing Xt's XtSetValues / XtGetValues.

    Resource& resources() { return resources_; }
    const Resource& resources() const { return resources_; }

    // Set a named resource value (type-safe)
    template <typename T>
    void setResource(const std::string& name, T value) {
        resources_.set<T>(name, std::move(value));
        onResourceChanged(name);
    }

    // Get a named resource value
    template <typename T>
    T getResource(const std::string& name, const T& def = T{}) const {
        return resources_.get<T>(name, def);
    }

    // Subclasses override to react to resource changes
    virtual void onResourceChanged(const std::string& resourceName);

    // Get the widget path for resource database lookup (e.g. "app.form.button")
    std::string widgetPath() const;

    // Apply default resources from the database for this widget's class
    virtual void initializeResources();

    // Returns the resource specs for this widget class
    virtual std::vector<ResourceSpec> resourceSpecs() const { return {}; }

    // Callbacks
    void addCallback(const std::string& reason, Callback cb);
    void removeCallbacks(const std::string& reason);

    // Event handlers (called by Application event dispatch)
    virtual void handleExpose();
    virtual void handleConfigure(int x, int y, int w, int h);
    virtual void handleMap();
    virtual void handleUnmap();
    virtual void handleButtonPress(unsigned int button, int x, int y);
    virtual void handleButtonRelease(unsigned int button, int x, int y);
    virtual void handleMotionNotify(int x, int y, unsigned int state);
    virtual void handleEnter(int x, int y);
    virtual void handleLeave();
    virtual void handleKeyPress(unsigned int keycode, unsigned int state);
    virtual void handleKeyRelease(unsigned int keycode, unsigned int state);
    virtual void handleFocusIn();
    virtual void handleFocusOut();
    virtual void handleDestroy();
    virtual void handleClose();

protected:
    virtual void expose();
    virtual void createWindow();
    virtual void destroyWindow();
    void invokeCallbacks(const std::string& reason, void* callData = nullptr);

    Widget* parent_ = nullptr;
    std::vector<Widget*> children_;

    XWindow window_ = 0;
    XGC gc_ = nullptr;

    int x_ = 0;
    int y_ = 0;
    int width_ = 0;
    int height_ = 0;

    bool realized_ = false;
    bool visible_ = true;
    bool sensitive_ = true;
    bool managed_ = false;
    std::string name_;
    Resource resources_;

private:
    struct CallbackEntry {
        std::string reason;
        Callback callback;
    };
    std::vector<CallbackEntry> callbacks_;
};

} // namespace motif
