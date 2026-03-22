#include <motif/dnd/DropSiteManager.h>
#include <motif/dnd/DragContext.h>
#include <motif/dnd/DragIcon.h>
#include <motif/core/Widget.h>
#include <motif/core/Application.h>

#include <X11/Xlib.h>
#include <algorithm>

namespace motif {

DropSiteManager& DropSiteManager::instance() {
    static DropSiteManager mgr;
    return mgr;
}

void DropSiteManager::registerDropSite(Widget* widget, const DropSite& site) {
    sites_[widget] = site;
    sites_[widget].widget = widget;
}

void DropSiteManager::unregisterDropSite(Widget* widget) {
    sites_.erase(widget);
    if (currentDropTarget_ == widget) {
        currentDropTarget_ = nullptr;
    }
}

DropSiteManager::DropSite* DropSiteManager::findDropSiteAt(int rootX, int rootY) {
    auto& app = Application::instance();
    auto* dpy = app.display();
    if (!dpy) return nullptr;

    // Use XTranslateCoordinates to find which widget is under the cursor
    Window rootWin = app.rootWindow();
    Window childWin = None;
    int destX, destY;

    for (auto& [widget, site] : sites_) {
        if (site.activity != DropActivity::Active) continue;
        if (!widget->window() || !widget->isRealized()) continue;

        XTranslateCoordinates(dpy, rootWin, widget->window(),
                              rootX, rootY, &destX, &destY, &childWin);

        if (destX >= 0 && destX < widget->width() &&
            destY >= 0 && destY < widget->height()) {
            return &site;
        }
    }
    return nullptr;
}

bool DropSiteManager::targetsMatch(const DragContext& ctx, const DropSite& site) const {
    for (const auto& exportTarget : ctx.exportTargets()) {
        for (const auto& importTarget : site.importTargets) {
            if (exportTarget == importTarget) return true;
        }
    }
    return false;
}

void DropSiteManager::handleDragMotion(DragContext& ctx, int rootX, int rootY) {
    DropSite* site = findDropSiteAt(rootX, rootY);
    Widget* newTarget = site ? site->widget : nullptr;

    if (newTarget != currentDropTarget_) {
        // Leave old target — remove highlight
        if (currentDropTarget_) {
            unhighlightDropSite(currentDropTarget_);
            auto it = sites_.find(currentDropTarget_);
            if (it != sites_.end() && it->second.dragLeaveCb) {
                it->second.dragLeaveCb();
            }
        }

        currentDropTarget_ = newTarget;

        // Enter new target — apply highlight and update drag icon state
        if (site) {
            bool accepted = targetsMatch(ctx, *site);
            if (highlightStyle_.initialized) {
                highlightDropSite(site->widget, accepted);
            }

            auto* icon = ctx.dragIcon();
            if (icon) {
                icon->setState(accepted ? DragIcon::State::Accept
                                        : DragIcon::State::Reject);
            }

            if (site->dragEnterCb && accepted) {
                site->dragEnterCb(ctx);
            }
        } else {
            // Not over any drop site — reset icon to idle
            auto* icon = ctx.dragIcon();
            if (icon) {
                icon->setState(DragIcon::State::Idle);
            }
        }
    }
}

bool DropSiteManager::handleDrop(DragContext& ctx, int rootX, int rootY) {
    DropSite* site = findDropSiteAt(rootX, rootY);

    // Remove highlight from current target
    if (currentDropTarget_) {
        unhighlightDropSite(currentDropTarget_);
    }
    currentDropTarget_ = nullptr;

    if (!site || !site->dropCb) return false;
    if (!targetsMatch(ctx, *site)) return false;

    // Find first matching target type and convert
    for (const auto& importTarget : site->importTargets) {
        for (const auto& exportTarget : ctx.exportTargets()) {
            if (exportTarget == importTarget) {
                std::string data;
                if (ctx.convertData(exportTarget, data)) {
                    return site->dropCb(ctx, data);
                }
            }
        }
    }

    return false;
}

void DropSiteManager::initHighlightColors(void* display, int screen) {
    auto* dpy = static_cast<Display*>(display);
    if (!dpy) return;

    Colormap cmap = DefaultColormap(dpy, screen);
    XColor color, exact;

    if (XAllocNamedColor(dpy, cmap, "#4CAF50", &color, &exact)) {
        highlightStyle_.acceptColor = color.pixel;
    }
    if (XAllocNamedColor(dpy, cmap, "#F44336", &color, &exact)) {
        highlightStyle_.rejectColor = color.pixel;
    }
    highlightStyle_.initialized = true;
}

void DropSiteManager::highlightDropSite(Widget* widget, bool accepted) {
    if (!widget || !widget->window() || !widget->isRealized()) return;

    auto* dpy = Application::instance().display();
    if (!dpy) return;

    // Save original border color if not already saved
    if (savedBorders_.find(widget) == savedBorders_.end()) {
        XWindowAttributes attrs;
        if (XGetWindowAttributes(dpy, widget->window(), &attrs)) {
            savedBorders_[widget] = attrs.border_width > 0
                ? static_cast<unsigned long>(attrs.border_width) : 0;
        }
    }

    unsigned long borderColor = accepted
        ? highlightStyle_.acceptColor
        : highlightStyle_.rejectColor;

    XSetWindowBorderWidth(dpy, widget->window(), highlightStyle_.borderWidth);
    XSetWindowBorder(dpy, widget->window(), borderColor);
}

void DropSiteManager::unhighlightDropSite(Widget* widget) {
    if (!widget || !widget->window() || !widget->isRealized()) return;

    auto* dpy = Application::instance().display();
    if (!dpy) return;

    auto it = savedBorders_.find(widget);
    if (it != savedBorders_.end()) {
        XSetWindowBorderWidth(dpy, widget->window(), static_cast<int>(it->second));
        savedBorders_.erase(it);
    } else {
        XSetWindowBorderWidth(dpy, widget->window(), 0);
    }

    XSetWindowBorder(dpy, widget->window(), Application::instance().blackPixel());
}

} // namespace motif
