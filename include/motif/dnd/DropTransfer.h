#pragma once

#include <functional>
#include <string>
#include <vector>

namespace motif {

class DragContext;
class Widget;

// Manages the data transfer phase after a successful drop.
// The drop site creates a DropTransfer to request conversions from the drag source.
class DropTransfer {
public:
    struct TransferEntry {
        std::string targetType;   // e.g. "STRING", "text/plain", "FILE_NAME"
    };

    using TransferCallback = std::function<void(const std::string& targetType,
                                                 const std::string& data,
                                                 bool success)>;

    explicit DropTransfer(DragContext& dragContext);
    ~DropTransfer();

    void addTransferEntry(const std::string& targetType);
    void setTransferCallback(TransferCallback cb) { transferCb_ = std::move(cb); }

    // Execute all transfers
    void startTransfer();

    // Cancel remaining transfers
    void cancelTransfer();

    bool isComplete() const { return complete_; }

private:
    DragContext& dragContext_;
    std::vector<TransferEntry> entries_;
    TransferCallback transferCb_;
    bool complete_ = false;
    bool cancelled_ = false;
};

} // namespace motif
