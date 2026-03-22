#pragma once

#include <motif/core/Widget.h>
#include <functional>

namespace motif {

class Shell : public Widget {
public:
    Shell() = default;
    explicit Shell(Widget* parent);
    ~Shell() override;

    void realize() override;

    void setTitle(const std::string& title);
    const std::string& title() const { return title_; }

    void setIconName(const std::string& name) { iconName_ = name; }
    const std::string& iconName() const { return iconName_; }

    void setMinSize(int w, int h) { minWidth_ = w; minHeight_ = h; }
    void setMaxSize(int w, int h) { maxWidth_ = w; maxHeight_ = h; }

    // Close callback: called when WM_DELETE_WINDOW is received
    void setCloseCallback(std::function<void()> cb) { closeCb_ = std::move(cb); }

    // Resource system
    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void handleClose() override;
    void handleConfigure(int x, int y, int w, int h) override;

protected:
    void createWindow() override;

    std::string title_;
    std::string iconName_;

    int minWidth_ = 0;
    int minHeight_ = 0;
    int maxWidth_ = 0;
    int maxHeight_ = 0;

    std::function<void()> closeCb_;
};

} // namespace motif
