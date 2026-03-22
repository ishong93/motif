#include <motif/core/Application.h>
#include <motif/core/Shell.h>
#include <motif/containers/Form.h>
#include <motif/widgets/Label.h>
#include <motif/widgets/TextField.h>
#include <motif/widgets/PushButton.h>

#include <iostream>

/// Demonstrates Form container with attachment-based layout.
/// Creates a login-style form: labels + text fields with OK/Cancel buttons.
int main() {
    using namespace motif;

    auto& app = Application::instance();
    if (!app.open()) return 1;

    Shell toplevel;
    toplevel.setTitle("Form Layout Example");
    toplevel.resize(360, 200);

    Form form(&toplevel);

    // --- Name row ---
    Label nameLabel(&form);
    nameLabel.setText("Name:");
    nameLabel.setAlignment(Label::Alignment::End);

    TextField nameField(&form);
    nameField.setColumns(25);

    Form::Constraint nameLabelC;
    nameLabelC.topAttachment  = Form::Attachment::Form;
    nameLabelC.leftAttachment = Form::Attachment::Form;
    nameLabelC.topOffset = 12;
    nameLabelC.leftOffset = 12;
    form.setConstraint(&nameLabel, nameLabelC);

    Form::Constraint nameFieldC;
    nameFieldC.topAttachment  = Form::Attachment::Form;
    nameFieldC.leftAttachment = Form::Attachment::Widget;
    nameFieldC.leftWidget     = &nameLabel;
    nameFieldC.rightAttachment = Form::Attachment::Form;
    nameFieldC.topOffset = 10;
    nameFieldC.leftOffset = 8;
    nameFieldC.rightOffset = 12;
    form.setConstraint(&nameField, nameFieldC);

    // --- Email row ---
    Label emailLabel(&form);
    emailLabel.setText("Email:");
    emailLabel.setAlignment(Label::Alignment::End);

    TextField emailField(&form);
    emailField.setColumns(25);

    Form::Constraint emailLabelC;
    emailLabelC.topAttachment  = Form::Attachment::Widget;
    emailLabelC.topWidget      = &nameLabel;
    emailLabelC.leftAttachment = Form::Attachment::Form;
    emailLabelC.topOffset = 10;
    emailLabelC.leftOffset = 12;
    form.setConstraint(&emailLabel, emailLabelC);

    Form::Constraint emailFieldC;
    emailFieldC.topAttachment  = Form::Attachment::Widget;
    emailFieldC.topWidget      = &nameField;
    emailFieldC.leftAttachment = Form::Attachment::Widget;
    emailFieldC.leftWidget     = &emailLabel;
    emailFieldC.rightAttachment = Form::Attachment::Form;
    emailFieldC.topOffset = 8;
    emailFieldC.leftOffset = 8;
    emailFieldC.rightOffset = 12;
    form.setConstraint(&emailField, emailFieldC);

    // --- Buttons ---
    PushButton okBtn(&form);
    okBtn.setText("OK");
    okBtn.addCallback(PushButton::ActivateCallback,
        [&](Widget&, void*) {
            std::cout << "Name:  " << nameField.value() << "\n"
                      << "Email: " << emailField.value() << "\n";
        });

    PushButton cancelBtn(&form);
    cancelBtn.setText("Cancel");
    cancelBtn.addCallback(PushButton::ActivateCallback,
        [](Widget&, void*) { Application::instance().quit(); });

    Form::Constraint okC;
    okC.bottomAttachment = Form::Attachment::Form;
    okC.leftAttachment   = Form::Attachment::Position;
    okC.leftPosition     = 20;
    okC.bottomOffset = 12;
    form.setConstraint(&okBtn, okC);

    Form::Constraint cancelC;
    cancelC.bottomAttachment = Form::Attachment::Form;
    cancelC.leftAttachment   = Form::Attachment::Position;
    cancelC.leftPosition     = 55;
    cancelC.bottomOffset = 12;
    form.setConstraint(&cancelBtn, cancelC);

    toplevel.realize();
    app.run();
    return 0;
}
