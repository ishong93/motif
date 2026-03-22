#pragma once

#include <motif/dialogs/DialogShell.h>
#include <motif/containers/Form.h>
#include <motif/widgets/Label.h>
#include <motif/widgets/PushButton.h>
#include <functional>
#include <string>
#include <vector>

namespace motif {

class SelectionBox {
public:
    SelectionBox() = default;
    explicit SelectionBox(Widget* parent);
    ~SelectionBox();

    void setTitle(const std::string& title);
    void setPrompt(const std::string& prompt);
    void setItems(const std::vector<std::string>& items);
    void setSelectedItem(const std::string& item);
    std::string selectedItem() const { return selectedItem_; }

    void setOkCallback(std::function<void(const std::string&)> cb) { okCb_ = std::move(cb); }
    void setCancelCallback(std::function<void()> cb) { cancelCb_ = std::move(cb); }

    void popup();
    void popdown();

    DialogShell& shell() { return shell_; }

private:
    void buildUI();

    DialogShell shell_;
    Form form_{&shell_};
    Label promptLabel_{&form_};
    Label listLabel_{&form_};    // displays items as text list (simplified)
    PushButton okButton_{&form_};
    PushButton cancelButton_{&form_};

    std::vector<std::string> items_;
    std::string selectedItem_;
    int selectedIndex_ = -1;
    bool built_ = false;

    std::function<void(const std::string&)> okCb_;
    std::function<void()> cancelCb_;
};

} // namespace motif
