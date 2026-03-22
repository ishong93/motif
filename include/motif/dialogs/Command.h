#pragma once

#include <motif/dialogs/DialogShell.h>
#include <motif/containers/Form.h>
#include <motif/widgets/Label.h>
#include <motif/widgets/PushButton.h>
#include <functional>
#include <string>
#include <vector>

namespace motif {

class CommandDialog {
public:
    CommandDialog() = default;
    explicit CommandDialog(Widget* parent);
    ~CommandDialog();

    void setTitle(const std::string& title);
    void setPrompt(const std::string& prompt);
    void setCommand(const std::string& command);
    std::string command() const { return command_; }

    const std::vector<std::string>& history() const { return history_; }

    void setCommandCallback(std::function<void(const std::string&)> cb) { commandCb_ = std::move(cb); }

    void popup();
    void popdown();

    DialogShell& shell() { return shell_; }

private:
    void buildUI();

    DialogShell shell_;
    Form form_{&shell_};
    Label promptLabel_{&form_};
    Label commandLabel_{&form_};
    Label historyLabel_{&form_};
    PushButton executeButton_{&form_};

    std::string command_;
    std::vector<std::string> history_;
    bool built_ = false;

    std::function<void(const std::string&)> commandCb_;
};

} // namespace motif
