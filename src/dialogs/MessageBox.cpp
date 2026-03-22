#include <motif/dialogs/MessageBox.h>
#include <motif/core/Application.h>

namespace motif {

MessageBox::MessageBox(Widget* parent) : shell_(parent) {
    shell_.resize(360, 160);
    shell_.setModal(true);
}

MessageBox::~MessageBox() = default;

void MessageBox::setTitle(const std::string& title) {
    shell_.setTitle(title);
}

void MessageBox::setMessage(const std::string& message) {
    messageLabel_.setText(message);
}

void MessageBox::setType(Type type) {
    type_ = type;
    switch (type) {
        case Type::Information: iconLabel_.setText("[i]"); break;
        case Type::Warning:     iconLabel_.setText("[!]"); break;
        case Type::Error:       iconLabel_.setText("[X]"); break;
        case Type::Question:    iconLabel_.setText("[?]"); break;
        case Type::Working:     iconLabel_.setText("[~]"); break;
    }
}

void MessageBox::setOkLabel(const std::string& label) {
    okButton_.setText(label);
}

void MessageBox::setCancelLabel(const std::string& label) {
    cancelButton_.setText(label);
}

void MessageBox::setHelpLabel(const std::string& label) {
    helpButton_.setText(label);
}

void MessageBox::buildUI() {
    if (built_) return;
    built_ = true;

    // Set defaults
    if (iconLabel_.text().empty()) setType(type_);
    if (okButton_.text().empty()) okButton_.setText("OK");
    if (cancelButton_.text().empty()) cancelButton_.setText("Cancel");
    if (helpButton_.text().empty()) helpButton_.setText("Help");

    iconLabel_.setAlignment(Label::Alignment::Center);
    messageLabel_.setAlignment(Label::Alignment::Center);

    // Layout with Form constraints
    // Icon: left side, top area
    Form::Constraint iconC;
    iconC.topAttachment = Form::Attachment::Form;
    iconC.leftAttachment = Form::Attachment::Form;
    iconC.bottomAttachment = Form::Attachment::Position;
    iconC.rightAttachment = Form::Attachment::Position;
    iconC.topOffset = 10;
    iconC.leftOffset = 10;
    iconC.bottomPosition = 60;
    iconC.rightPosition = 20;
    form_.setConstraint(&iconLabel_, iconC);

    // Message: right of icon, top area
    Form::Constraint msgC;
    msgC.topAttachment = Form::Attachment::Form;
    msgC.leftAttachment = Form::Attachment::Widget;
    msgC.leftWidget = &iconLabel_;
    msgC.rightAttachment = Form::Attachment::Form;
    msgC.bottomAttachment = Form::Attachment::Position;
    msgC.topOffset = 10;
    msgC.leftOffset = 10;
    msgC.rightOffset = 10;
    msgC.bottomPosition = 60;
    form_.setConstraint(&messageLabel_, msgC);

    // Separator: full width at 65%
    Form::Constraint sepC;
    sepC.topAttachment = Form::Attachment::Position;
    sepC.leftAttachment = Form::Attachment::Form;
    sepC.rightAttachment = Form::Attachment::Form;
    sepC.topPosition = 62;
    sepC.leftOffset = 5;
    sepC.rightOffset = 5;
    form_.setConstraint(&separator_, sepC);
    separator_.setHighlightThickness(0);
    separator_.setShadowThickness(1);

    // Buttons: bottom area
    int numButtons = 1 + (showCancel_ ? 1 : 0) + (showHelp_ ? 1 : 0);
    int btnWidth = 100 / (numButtons + 1); // percentage spacing

    // OK button
    Form::Constraint okC;
    okC.topAttachment = Form::Attachment::Position;
    okC.topPosition = 68;
    okC.bottomAttachment = Form::Attachment::Form;
    okC.bottomOffset = 10;
    okC.leftAttachment = Form::Attachment::Position;
    okC.rightAttachment = Form::Attachment::Position;
    okC.leftPosition = 5;
    okC.rightPosition = showCancel_ ? 33 : (showHelp_ ? 33 : 95);
    form_.setConstraint(&okButton_, okC);

    okButton_.addCallback(PushButton::ActivateCallback, [this](Widget&, void*) {
        if (okCb_) okCb_();
        popdown();
    });

    if (showCancel_) {
        Form::Constraint cancelC;
        cancelC.topAttachment = Form::Attachment::Position;
        cancelC.topPosition = 68;
        cancelC.bottomAttachment = Form::Attachment::Form;
        cancelC.bottomOffset = 10;
        cancelC.leftAttachment = Form::Attachment::Position;
        cancelC.rightAttachment = Form::Attachment::Position;
        cancelC.leftPosition = 36;
        cancelC.rightPosition = showHelp_ ? 64 : 95;
        form_.setConstraint(&cancelButton_, cancelC);

        cancelButton_.addCallback(PushButton::ActivateCallback, [this](Widget&, void*) {
            if (cancelCb_) cancelCb_();
            popdown();
        });
    } else {
        cancelButton_.hide();
    }

    if (showHelp_) {
        Form::Constraint helpC;
        helpC.topAttachment = Form::Attachment::Position;
        helpC.topPosition = 68;
        helpC.bottomAttachment = Form::Attachment::Form;
        helpC.bottomOffset = 10;
        helpC.leftAttachment = Form::Attachment::Position;
        helpC.rightAttachment = Form::Attachment::Position;
        helpC.leftPosition = 67;
        helpC.rightPosition = 95;
        form_.setConstraint(&helpButton_, helpC);

        helpButton_.addCallback(PushButton::ActivateCallback, [this](Widget&, void*) {
            if (helpCb_) helpCb_();
        });
    } else {
        helpButton_.hide();
    }

    // Shell close → cancel
    shell_.setCloseCallback([this]() {
        if (cancelCb_) cancelCb_();
        popdown();
    });
}

void MessageBox::popup() {
    buildUI();
    shell_.popup();
}

void MessageBox::popdown() {
    shell_.popdown();
}

} // namespace motif
