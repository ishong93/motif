#include <motif/dnd/DropTransfer.h>
#include <motif/dnd/DragContext.h>

namespace motif {

DropTransfer::DropTransfer(DragContext& dragContext)
    : dragContext_(dragContext) {}

DropTransfer::~DropTransfer() = default;

void DropTransfer::addTransferEntry(const std::string& targetType) {
    entries_.push_back({targetType});
}

void DropTransfer::startTransfer() {
    if (complete_ || cancelled_) return;

    for (const auto& entry : entries_) {
        if (cancelled_) break;

        std::string data;
        bool success = dragContext_.convertData(entry.targetType, data);

        if (transferCb_) {
            transferCb_(entry.targetType, data, success);
        }
    }

    complete_ = true;
}

void DropTransfer::cancelTransfer() {
    cancelled_ = true;
    complete_ = true;

    if (transferCb_) {
        transferCb_("", "", false);
    }
}

} // namespace motif
