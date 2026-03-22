#pragma once

#include <motif/dialogs/DialogShell.h>
#include <motif/containers/Form.h>
#include <motif/widgets/Label.h>
#include <motif/widgets/PushButton.h>
#include <functional>
#include <string>
#include <vector>

namespace motif {

class FileSelectionBox {
public:
    FileSelectionBox() = default;
    explicit FileSelectionBox(Widget* parent);
    ~FileSelectionBox();

    void setTitle(const std::string& title);
    void setDirectory(const std::string& dir);
    void setPattern(const std::string& pattern);
    std::string selectedFile() const { return selectedFile_; }

    void setOkCallback(std::function<void(const std::string&)> cb) { okCb_ = std::move(cb); }
    void setCancelCallback(std::function<void()> cb) { cancelCb_ = std::move(cb); }

    void popup();
    void popdown();

    DialogShell& shell() { return shell_; }

private:
    void buildUI();
    void scanDirectory();

    DialogShell shell_;
    Form form_{&shell_};
    Label filterLabel_{&form_};
    Label dirLabel_{&form_};
    Label fileListLabel_{&form_};
    Label selectionLabel_{&form_};
    PushButton okButton_{&form_};
    PushButton cancelButton_{&form_};
    PushButton filterButton_{&form_};

    std::string directory_ = ".";
    std::string pattern_ = "*";
    std::string selectedFile_;
    std::vector<std::string> files_;
    std::vector<std::string> dirs_;
    int selectedIndex_ = -1;
    bool built_ = false;

    std::function<void(const std::string&)> okCb_;
    std::function<void()> cancelCb_;
};

} // namespace motif
