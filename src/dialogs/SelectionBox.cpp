#include <motif/dialogs/SelectionBox.h>
#include <motif/core/Application.h>

namespace motif {

SelectionBox::SelectionBox(Widget* parent) : shell_(parent) {
    shell_.resize(300, 280);
    shell_.setModal(true);
}

SelectionBox::~SelectionBox() = default;

void SelectionBox::setTitle(const std::string& title) {
    shell_.setTitle(title);
}

void SelectionBox::setPrompt(const std::string& prompt) {
    promptLabel_.setText(prompt);
}

void SelectionBox::setItems(const std::vector<std::string>& items) {
    items_ = items;
    // Build display string
    std::string display;
    for (size_t i = 0; i < items_.size(); ++i) {
        if (i == static_cast<size_t>(selectedIndex_)) {
            display += "> ";
        } else {
            display += "  ";
        }
        display += items_[i];
        if (i + 1 < items_.size()) display += "  |  ";
    }
    listLabel_.setText(display);
}

void SelectionBox::setSelectedItem(const std::string& item) {
    selectedItem_ = item;
    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i] == item) {
            selectedIndex_ = static_cast<int>(i);
            break;
        }
    }
    setItems(items_); // refresh display
}

void SelectionBox::buildUI() {
    if (built_) return;
    built_ = true;

    promptLabel_.setAlignment(Label::Alignment::Beginning);
    listLabel_.setAlignment(Label::Alignment::Beginning);

    if (okButton_.text().empty()) okButton_.setText("OK");
    if (cancelButton_.text().empty()) cancelButton_.setText("Cancel");

    // Prompt at top
    Form::Constraint promptC;
    promptC.topAttachment = Form::Attachment::Form;
    promptC.leftAttachment = Form::Attachment::Form;
    promptC.rightAttachment = Form::Attachment::Form;
    promptC.topOffset = 10;
    promptC.leftOffset = 10;
    promptC.rightOffset = 10;
    form_.setConstraint(&promptLabel_, promptC);

    // List area
    Form::Constraint listC;
    listC.topAttachment = Form::Attachment::Widget;
    listC.topWidget = &promptLabel_;
    listC.leftAttachment = Form::Attachment::Form;
    listC.rightAttachment = Form::Attachment::Form;
    listC.bottomAttachment = Form::Attachment::Position;
    listC.topOffset = 5;
    listC.leftOffset = 10;
    listC.rightOffset = 10;
    listC.bottomPosition = 75;
    form_.setConstraint(&listLabel_, listC);

    // Clicking on list cycles selection
    listLabel_.addCallback("activate", [this](Widget&, void*) {
        if (!items_.empty()) {
            selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(items_.size());
            selectedItem_ = items_[selectedIndex_];
            setItems(items_);
        }
    });

    // OK button
    Form::Constraint okC;
    okC.topAttachment = Form::Attachment::Position;
    okC.topPosition = 80;
    okC.bottomAttachment = Form::Attachment::Form;
    okC.bottomOffset = 10;
    okC.leftAttachment = Form::Attachment::Position;
    okC.rightAttachment = Form::Attachment::Position;
    okC.leftPosition = 10;
    okC.rightPosition = 48;
    form_.setConstraint(&okButton_, okC);

    okButton_.addCallback(PushButton::ActivateCallback, [this](Widget&, void*) {
        if (okCb_) okCb_(selectedItem_);
        popdown();
    });

    // Cancel button
    Form::Constraint cancelC;
    cancelC.topAttachment = Form::Attachment::Position;
    cancelC.topPosition = 80;
    cancelC.bottomAttachment = Form::Attachment::Form;
    cancelC.bottomOffset = 10;
    cancelC.leftAttachment = Form::Attachment::Position;
    cancelC.rightAttachment = Form::Attachment::Position;
    cancelC.leftPosition = 52;
    cancelC.rightPosition = 90;
    form_.setConstraint(&cancelButton_, cancelC);

    cancelButton_.addCallback(PushButton::ActivateCallback, [this](Widget&, void*) {
        if (cancelCb_) cancelCb_();
        popdown();
    });

    shell_.setCloseCallback([this]() {
        if (cancelCb_) cancelCb_();
        popdown();
    });
}

void SelectionBox::popup() {
    buildUI();
    shell_.popup();
}

void SelectionBox::popdown() {
    shell_.popdown();
}

} // namespace motif
