#include <motif/dialogs/Command.h>

namespace motif {

CommandDialog::CommandDialog(Widget* parent) : shell_(parent) {
    shell_.resize(350, 250);
    shell_.setModal(false);
}

CommandDialog::~CommandDialog() = default;

void CommandDialog::setTitle(const std::string& title) {
    shell_.setTitle(title);
}

void CommandDialog::setPrompt(const std::string& prompt) {
    promptLabel_.setText(prompt);
}

void CommandDialog::setCommand(const std::string& command) {
    command_ = command;
    commandLabel_.setText(command_);
}

void CommandDialog::buildUI() {
    if (built_) return;
    built_ = true;

    if (promptLabel_.text().empty()) promptLabel_.setText("Command:");
    promptLabel_.setAlignment(Label::Alignment::Beginning);
    commandLabel_.setAlignment(Label::Alignment::Beginning);
    historyLabel_.setAlignment(Label::Alignment::Beginning);
    executeButton_.setText("Execute");

    // Prompt
    Form::Constraint promptC;
    promptC.topAttachment = Form::Attachment::Form;
    promptC.leftAttachment = Form::Attachment::Form;
    promptC.rightAttachment = Form::Attachment::Form;
    promptC.topOffset = 8;
    promptC.leftOffset = 8;
    promptC.rightOffset = 8;
    form_.setConstraint(&promptLabel_, promptC);

    // Command input area
    Form::Constraint cmdC;
    cmdC.topAttachment = Form::Attachment::Widget;
    cmdC.topWidget = &promptLabel_;
    cmdC.leftAttachment = Form::Attachment::Form;
    cmdC.rightAttachment = Form::Attachment::Form;
    cmdC.topOffset = 4;
    cmdC.leftOffset = 8;
    cmdC.rightOffset = 8;
    form_.setConstraint(&commandLabel_, cmdC);

    // History area
    Form::Constraint histC;
    histC.topAttachment = Form::Attachment::Widget;
    histC.topWidget = &commandLabel_;
    histC.leftAttachment = Form::Attachment::Form;
    histC.rightAttachment = Form::Attachment::Form;
    histC.bottomAttachment = Form::Attachment::Position;
    histC.topOffset = 8;
    histC.leftOffset = 8;
    histC.rightOffset = 8;
    histC.bottomPosition = 80;
    form_.setConstraint(&historyLabel_, histC);

    // Execute button
    Form::Constraint execC;
    execC.topAttachment = Form::Attachment::Position;
    execC.topPosition = 82;
    execC.bottomAttachment = Form::Attachment::Form;
    execC.bottomOffset = 8;
    execC.leftAttachment = Form::Attachment::Position;
    execC.rightAttachment = Form::Attachment::Position;
    execC.leftPosition = 30;
    execC.rightPosition = 70;
    form_.setConstraint(&executeButton_, execC);

    executeButton_.addCallback(PushButton::ActivateCallback, [this](Widget&, void*) {
        if (!command_.empty()) {
            history_.push_back(command_);
            // Update history display
            std::string histDisplay;
            for (auto it = history_.rbegin(); it != history_.rend(); ++it) {
                histDisplay += *it + "\n";
            }
            historyLabel_.setText(histDisplay);

            if (commandCb_) commandCb_(command_);
        }
    });

    shell_.setCloseCallback([this]() { popdown(); });
}

void CommandDialog::popup() {
    buildUI();
    shell_.popup();
}

void CommandDialog::popdown() {
    shell_.popdown();
}

} // namespace motif
