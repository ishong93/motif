#pragma once

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace motif {

class Widget;

// ── SelectionAtoms: cached X11 atoms for selection protocol ──────────

struct SelectionAtoms {
    Atom clipboard    = None;
    Atom primary      = XA_PRIMARY;
    Atom targets      = None;
    Atom multiple     = None;
    Atom timestamp    = None;
    Atom utf8String   = None;
    Atom compoundText = None;
    Atom textPlain    = None;
    Atom motifDrop    = None;

    void initialize(Display* dpy);
    static SelectionAtoms& instance();
};

// ── TransferRequest: one pending async data request ──────────────────

struct TransferRequest {
    using Callback = std::function<void(const std::string& type,
                                         const uint8_t* data, size_t size,
                                         bool success)>;

    Atom selection = None;     // CLIPBOARD or PRIMARY
    Atom target    = None;     // UTF8_STRING, TARGETS, etc.
    Atom property  = None;     // property on requestor window
    Window requestor = None;
    Callback callback;
    uint64_t id = 0;
};

// ── SelectionOwnership: tracks what we offer when we own a selection ─

struct SelectionOffer {
    using ConvertCallback = std::function<bool(Atom target,
                                                std::vector<uint8_t>& dataOut,
                                                Atom& typeOut)>;

    Atom selection = None;
    Window owner = None;
    std::vector<Atom> supportedTargets;
    ConvertCallback convertCb;
    std::function<void()> lostCb;    // called when ownership lost
};

// ── TransferManager ──────────────────────────────────────────────────
// Central coordinator for X Selection protocol.
// Replaces the tangled Transfer.c / CutPaste.c / SelectioB.c code.
//
// Key design differences from legacy Motif:
//   1. Fully async — callbacks fire when SelectionNotify arrives
//   2. No global locks — concurrent selections tracked by ID
//   3. Supports TARGETS negotiation and MULTIPLE transfers
//   4. UTF-8 as primary text encoding (not compound text)

class TransferManager {
public:
    static TransferManager& instance();

    void initialize(Display* dpy);

    // ── Clipboard operations (high-level) ────────────────────────
    bool copyText(Window owner, const std::string& text);
    void pasteText(Window requestor, std::function<void(const std::string&)> callback);

    // ── Selection ownership ──────────────────────────────────────
    bool ownSelection(const SelectionOffer& offer);
    void disownSelection(Atom selection);
    bool isOwner(Atom selection) const;

    // ── Selection requests (outgoing) ────────────────────────────
    uint64_t requestSelection(Atom selection, Atom target,
                               Window requestor, TransferRequest::Callback cb);

    // Request TARGETS first, then pick the best one
    void requestBestText(Atom selection, Window requestor,
                          std::function<void(const std::string&)> cb);

    // ── Event handlers (called from Application event loop) ──────
    void handleSelectionRequest(const XSelectionRequestEvent& ev);
    void handleSelectionNotify(const XSelectionEvent& ev);
    void handleSelectionClear(const XSelectionClearEvent& ev);

private:
    TransferManager() = default;

    Display* display_ = nullptr;
    uint64_t nextRequestId_ = 1;

    // Pending incoming data requests (we asked someone else for data)
    std::unordered_map<Atom, TransferRequest> pendingRequests_;  // keyed by property atom

    // Selections we currently own
    std::unordered_map<Atom, SelectionOffer> ownedSelections_;   // keyed by selection atom

    // Clipboard data (when we own CLIPBOARD)
    std::string clipboardText_;
};

} // namespace motif
