#include <motif/dialogs/FileSelectionBox.h>
#include <motif/core/Application.h>

#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <cstring>
#include <fnmatch.h>

namespace motif {

FileSelectionBox::FileSelectionBox(Widget* parent) : shell_(parent) {
    shell_.resize(400, 360);
    shell_.setModal(true);
}

FileSelectionBox::~FileSelectionBox() = default;

void FileSelectionBox::setTitle(const std::string& title) {
    shell_.setTitle(title);
}

void FileSelectionBox::setDirectory(const std::string& dir) {
    directory_ = dir;
    scanDirectory();
}

void FileSelectionBox::setPattern(const std::string& pattern) {
    pattern_ = pattern;
    scanDirectory();
}

void FileSelectionBox::scanDirectory() {
    files_.clear();
    dirs_.clear();

    DIR* d = opendir(directory_.c_str());
    if (!d) return;

    struct dirent* entry;
    while ((entry = readdir(d)) != nullptr) {
        std::string name = entry->d_name;

        std::string fullPath = directory_ + "/" + name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            dirs_.push_back(name + "/");
        } else {
            if (fnmatch(pattern_.c_str(), name.c_str(), 0) == 0) {
                files_.push_back(name);
            }
        }
    }
    closedir(d);

    std::sort(dirs_.begin(), dirs_.end());
    std::sort(files_.begin(), files_.end());

    // Update display
    std::string dirDisplay;
    for (auto& dn : dirs_) dirDisplay += dn + "  ";
    dirLabel_.setText(dirDisplay.empty() ? "(empty)" : dirDisplay);

    std::string fileDisplay;
    for (size_t i = 0; i < files_.size(); ++i) {
        if (static_cast<int>(i) == selectedIndex_) {
            fileDisplay += "> ";
        } else {
            fileDisplay += "  ";
        }
        fileDisplay += files_[i] + "  ";
    }
    fileListLabel_.setText(fileDisplay.empty() ? "(no files)" : fileDisplay);

    selectionLabel_.setText(selectedFile_);
}

void FileSelectionBox::buildUI() {
    if (built_) return;
    built_ = true;

    filterLabel_.setText(directory_ + "/" + pattern_);
    filterLabel_.setAlignment(Label::Alignment::Beginning);
    dirLabel_.setAlignment(Label::Alignment::Beginning);
    fileListLabel_.setAlignment(Label::Alignment::Beginning);
    selectionLabel_.setAlignment(Label::Alignment::Beginning);

    okButton_.setText("OK");
    cancelButton_.setText("Cancel");
    filterButton_.setText("Filter");

    // Filter path at top
    Form::Constraint filterC;
    filterC.topAttachment = Form::Attachment::Form;
    filterC.leftAttachment = Form::Attachment::Form;
    filterC.rightAttachment = Form::Attachment::Form;
    filterC.topOffset = 8;
    filterC.leftOffset = 8;
    filterC.rightOffset = 8;
    form_.setConstraint(&filterLabel_, filterC);

    // Directory list
    Form::Constraint dirC;
    dirC.topAttachment = Form::Attachment::Widget;
    dirC.topWidget = &filterLabel_;
    dirC.leftAttachment = Form::Attachment::Form;
    dirC.rightAttachment = Form::Attachment::Position;
    dirC.bottomAttachment = Form::Attachment::Position;
    dirC.topOffset = 4;
    dirC.leftOffset = 8;
    dirC.rightPosition = 48;
    dirC.bottomPosition = 60;
    form_.setConstraint(&dirLabel_, dirC);

    // File list
    Form::Constraint fileC;
    fileC.topAttachment = Form::Attachment::Widget;
    fileC.topWidget = &filterLabel_;
    fileC.leftAttachment = Form::Attachment::Position;
    fileC.rightAttachment = Form::Attachment::Form;
    fileC.bottomAttachment = Form::Attachment::Position;
    fileC.topOffset = 4;
    fileC.leftPosition = 52;
    fileC.rightOffset = 8;
    fileC.bottomPosition = 60;
    form_.setConstraint(&fileListLabel_, fileC);

    // Clicking file list cycles selection
    fileListLabel_.addCallback("activate", [this](Widget&, void*) {
        if (!files_.empty()) {
            selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(files_.size());
            selectedFile_ = directory_ + "/" + files_[selectedIndex_];
            scanDirectory();
        }
    });

    // Selection display
    Form::Constraint selC;
    selC.topAttachment = Form::Attachment::Position;
    selC.topPosition = 62;
    selC.leftAttachment = Form::Attachment::Form;
    selC.rightAttachment = Form::Attachment::Form;
    selC.leftOffset = 8;
    selC.rightOffset = 8;
    form_.setConstraint(&selectionLabel_, selC);

    // Buttons at bottom
    Form::Constraint okC;
    okC.topAttachment = Form::Attachment::Position;
    okC.topPosition = 80;
    okC.bottomAttachment = Form::Attachment::Form;
    okC.bottomOffset = 8;
    okC.leftAttachment = Form::Attachment::Position;
    okC.rightAttachment = Form::Attachment::Position;
    okC.leftPosition = 3;
    okC.rightPosition = 30;
    form_.setConstraint(&okButton_, okC);

    Form::Constraint filterBtnC;
    filterBtnC.topAttachment = Form::Attachment::Position;
    filterBtnC.topPosition = 80;
    filterBtnC.bottomAttachment = Form::Attachment::Form;
    filterBtnC.bottomOffset = 8;
    filterBtnC.leftAttachment = Form::Attachment::Position;
    filterBtnC.rightAttachment = Form::Attachment::Position;
    filterBtnC.leftPosition = 35;
    filterBtnC.rightPosition = 65;
    form_.setConstraint(&filterButton_, filterBtnC);

    Form::Constraint cancelC;
    cancelC.topAttachment = Form::Attachment::Position;
    cancelC.topPosition = 80;
    cancelC.bottomAttachment = Form::Attachment::Form;
    cancelC.bottomOffset = 8;
    cancelC.leftAttachment = Form::Attachment::Position;
    cancelC.rightAttachment = Form::Attachment::Position;
    cancelC.leftPosition = 70;
    cancelC.rightPosition = 97;
    form_.setConstraint(&cancelButton_, cancelC);

    okButton_.addCallback(PushButton::ActivateCallback, [this](Widget&, void*) {
        if (okCb_) okCb_(selectedFile_);
        popdown();
    });

    cancelButton_.addCallback(PushButton::ActivateCallback, [this](Widget&, void*) {
        if (cancelCb_) cancelCb_();
        popdown();
    });

    filterButton_.addCallback(PushButton::ActivateCallback, [this](Widget&, void*) {
        scanDirectory();
    });

    shell_.setCloseCallback([this]() {
        if (cancelCb_) cancelCb_();
        popdown();
    });

    scanDirectory();
}

void FileSelectionBox::popup() {
    buildUI();
    shell_.popup();
}

void FileSelectionBox::popdown() {
    shell_.popdown();
}

} // namespace motif
