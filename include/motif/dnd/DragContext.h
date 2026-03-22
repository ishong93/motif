#pragma once

#include <motif/core/Widget.h>
#include <X11/Xlib.h>
#include <functional>
#include <string>
#include <vector>

namespace motif {

class DragIcon;
class DropSiteManager;

// ── XDND Protocol Atoms ──────────────────────────────────────────────

struct XdndAtoms {
    Atom XdndAware       = None;
    Atom XdndEnter       = None;
    Atom XdndPosition    = None;
    Atom XdndStatus      = None;
    Atom XdndLeave       = None;
    Atom XdndDrop        = None;
    Atom XdndFinished    = None;
    Atom XdndSelection   = None;
    Atom XdndTypeList    = None;
    Atom XdndActionCopy  = None;
    Atom XdndActionMove  = None;
    Atom XdndActionLink  = None;
    Atom XdndProxy       = None;

    void initialize(Display* dpy);
    static XdndAtoms& instance();
};

// ── DragContext ──────────────────────────────────────────────────────
// Manages a single drag operation.  Supports both:
//   - Internal motif DnD (via DropSiteManager)
//   - XDND protocol (inter-client drag-and-drop)
//
// Key redesign from legacy Motif DragC.c:
//   1. No Xt translation tables — direct event handling
//   2. Uses standard XDND v5 protocol instead of Motif DnD protocol
//   3. DragIcon is optional overlay window (not required for XDND)
//   4. Atom-based types alongside string types for X inter-client

class DragContext {
public:
    enum class DragAction { Copy, Move, Link };
    enum class Protocol { Internal, Xdnd };

    using ConvertCallback = std::function<bool(const std::string& targetType, std::string& dataOut)>;
    using FinishCallback  = std::function<void(DragAction action, bool success)>;

    DragContext(Widget* source, const std::vector<std::string>& exportTargets);
    ~DragContext();

    // Start the drag (call from ButtonPress or MotionNotify handler)
    void startDrag(int rootX, int rootY);
    void cancelDrag();
    bool isDragging() const { return dragging_; }

    Widget* source() const { return source_; }
    const std::vector<std::string>& exportTargets() const { return exportTargets_; }

    void setDragAction(DragAction action) { action_ = action; }
    DragAction dragAction() const { return action_; }

    void setConvertCallback(ConvertCallback cb) { convertCb_ = std::move(cb); }
    void setFinishCallback(FinishCallback cb) { finishCb_ = std::move(cb); }

    void setDragIcon(DragIcon* icon) { dragIcon_ = icon; }
    DragIcon* dragIcon() const { return dragIcon_; }

    // Threshold: pixels of motion before drag starts
    void setDragThreshold(int px) { threshold_ = px; }
    int dragThreshold() const { return threshold_; }

    // Called internally by event system
    void handleMotion(int rootX, int rootY);
    void handleDrop(int rootX, int rootY);

    // Data conversion (called by DropSiteManager or XDND SelectionRequest)
    bool convertData(const std::string& targetType, std::string& dataOut) const;

    // XDND event handlers (called from Application event dispatch)
    void handleXdndStatus(const XClientMessageEvent& ev);
    void handleXdndFinished(const XClientMessageEvent& ev);
    bool handleSelectionRequest(const XSelectionRequestEvent& ev);

    Protocol protocol() const { return protocol_; }

private:
    void sendXdndEnter(Window target);
    void sendXdndPosition(Window target, int rootX, int rootY);
    void sendXdndLeave(Window target);
    void sendXdndDrop(Window target);

    Window findXdndAwareWindow(int rootX, int rootY) const;
    Atom actionToXdndAtom() const;

    Widget* source_;
    std::vector<std::string> exportTargets_;
    std::vector<Atom> exportAtoms_;  // cached Atom versions of exportTargets

    DragAction action_ = DragAction::Copy;
    Protocol protocol_ = Protocol::Internal;
    bool dragging_ = false;
    bool thresholdMet_ = false;

    int startX_ = 0;
    int startY_ = 0;
    int threshold_ = 5;

    // XDND state
    Window xdndTarget_ = None;       // current XDND-aware target window
    bool xdndAccepted_ = false;      // target sent XdndStatus with accept

    DragIcon* dragIcon_ = nullptr;
    ConvertCallback convertCb_;
    FinishCallback finishCb_;
};

} // namespace motif
