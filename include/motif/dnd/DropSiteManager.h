#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace motif {

class Widget;
class DragContext;

class DropSiteManager {
public:
    static DropSiteManager& instance();

    enum class DropActivity { Active, Inactive };

    struct DropSite {
        Widget* widget = nullptr;
        std::vector<std::string> importTargets;
        DropActivity activity = DropActivity::Active;

        using DragEnterCallback = std::function<void(DragContext&)>;
        using DragLeaveCallback = std::function<void()>;
        using DropCallback = std::function<bool(DragContext&, const std::string& data)>;

        DragEnterCallback dragEnterCb;
        DragLeaveCallback dragLeaveCb;
        DropCallback dropCb;
    };

    void registerDropSite(Widget* widget, const DropSite& site);
    void unregisterDropSite(Widget* widget);

    // Called by DragContext during drag
    void handleDragMotion(DragContext& ctx, int rootX, int rootY);
    bool handleDrop(DragContext& ctx, int rootX, int rootY);

    /// Drop site highlight configuration
    struct HighlightStyle {
        int borderWidth = 2;
        unsigned long acceptColor = 0;   // green highlight
        unsigned long rejectColor = 0;   // red highlight
        bool initialized = false;
    };

    /// Initialize highlight colors (call after display open)
    void initHighlightColors(void* display, int screen);

private:
    DropSiteManager() = default;

    DropSite* findDropSiteAt(int rootX, int rootY);
    bool targetsMatch(const DragContext& ctx, const DropSite& site) const;

    void highlightDropSite(Widget* widget, bool accepted);
    void unhighlightDropSite(Widget* widget);

    std::unordered_map<Widget*, DropSite> sites_;
    std::unordered_map<Widget*, unsigned long> savedBorders_; // original border colors
    Widget* currentDropTarget_ = nullptr;
    HighlightStyle highlightStyle_;
};

} // namespace motif
