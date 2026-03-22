#pragma once

#include <motif/dialogs/DialogShell.h>
#include <motif/containers/Form.h>
#include <motif/widgets/Label.h>
#include <motif/widgets/PushButton.h>
#include <string>
#include <functional>

namespace motif {

class MessageBox {
public:
    enum class Type { Information, Warning, Error, Question, Working };

    static constexpr const char* OkCallback     = "ok";
    static constexpr const char* CancelCallback = "cancel";
    static constexpr const char* HelpCallback   = "help";

    MessageBox() = default;
    explicit MessageBox(Widget* parent);
    ~MessageBox();

    void setTitle(const std::string& title);
    void setMessage(const std::string& message);
    void setType(Type type);

    void setOkLabel(const std::string& label);
    void setCancelLabel(const std::string& label);
    void setHelpLabel(const std::string& label);

    void setShowCancel(bool show) { showCancel_ = show; }
    void setShowHelp(bool show) { showHelp_ = show; }

    void setOkCallback(std::function<void()> cb) { okCb_ = std::move(cb); }
    void setCancelCallback(std::function<void()> cb) { cancelCb_ = std::move(cb); }
    void setHelpCallback(std::function<void()> cb) { helpCb_ = std::move(cb); }

    void popup();
    void popdown();

    // Access child widgets for customization
    DialogShell& shell() { return shell_; }
    Label& messageLabel() { return messageLabel_; }
    PushButton& okButton() { return okButton_; }
    PushButton& cancelButton() { return cancelButton_; }
    PushButton& helpButton() { return helpButton_; }

private:
    void buildUI();

    DialogShell shell_;
    Form form_{&shell_};
    Label iconLabel_{&form_};
    Label messageLabel_{&form_};
    Label separator_{&form_};
    PushButton okButton_{&form_};
    PushButton cancelButton_{&form_};
    PushButton helpButton_{&form_};

    Type type_ = Type::Information;
    bool showCancel_ = true;
    bool showHelp_ = false;
    bool built_ = false;

    std::function<void()> okCb_;
    std::function<void()> cancelCb_;
    std::function<void()> helpCb_;
};

} // namespace motif
