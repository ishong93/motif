#include <motif/clipboard/CutPaste.h>
#include <motif/clipboard/Transfer.h>
#include <X11/Xatom.h>

namespace motif {
namespace clipboard {

bool copyText(Display* dpy, Window window, const std::string& text) {
    if (!dpy) return false;
    return TransferManager::instance().copyText(window, text);
}

void pasteText(Display* dpy, Window window, PasteCallback callback) {
    if (!dpy) return;
    TransferManager::instance().pasteText(window, std::move(callback));
}

bool lock(Display* /*dpy*/, Window /*window*/) {
    // X11 selections are inherently atomic per-request.
    // This is a compatibility stub for the legacy Motif API.
    return true;
}

void unlock(Display* /*dpy*/, Window /*window*/) {
    // No-op — see lock()
}

bool store(Display* dpy, Window window,
           const std::string& format, const std::string& data) {
    if (!dpy) return false;

    auto& atoms = SelectionAtoms::instance();
    Atom formatAtom = XInternAtom(dpy, format.c_str(), False);

    SelectionOffer offer;
    offer.selection = atoms.clipboard;
    offer.owner = window;
    offer.supportedTargets = {atoms.targets, formatAtom};
    offer.convertCb = [data, formatAtom, &atoms](Atom target,
                                                   std::vector<uint8_t>& dataOut,
                                                   Atom& typeOut) -> bool {
        if (target == atoms.targets) {
            std::vector<Atom> tgts = {atoms.targets, formatAtom};
            dataOut.resize(tgts.size() * sizeof(Atom));
            std::memcpy(dataOut.data(), tgts.data(), dataOut.size());
            typeOut = XA_ATOM;
            return true;
        }
        if (target == formatAtom || target == XA_STRING) {
            dataOut.assign(data.begin(), data.end());
            typeOut = formatAtom;
            return true;
        }
        return false;
    };

    return TransferManager::instance().ownSelection(offer);
}

bool retrieve(Display* dpy, Window window,
              const std::string& format, std::string& dataOut) {
    if (!dpy) return false;

    auto& atoms = SelectionAtoms::instance();

    // If we own the clipboard, short-circuit synchronously
    if (TransferManager::instance().isOwner(atoms.clipboard)) {
        // The TransferManager doesn't expose raw data for arbitrary formats
        // in a sync API. For self-owned selections, trigger via pasteText.
        // This is a best-effort synchronous path.
        bool received = false;
        TransferManager::instance().pasteText(window,
            [&dataOut, &received](const std::string& text) {
                dataOut = text;
                received = true;
            });
        return received;
    }

    // For external owners, async retrieval is needed.
    // Return false to indicate caller should use pasteText() instead.
    return false;
}

} // namespace clipboard
} // namespace motif
