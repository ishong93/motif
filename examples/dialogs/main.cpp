#include <motif/core/Application.h>
#include <motif/core/Shell.h>
#include <motif/containers/RowColumn.h>
#include <motif/widgets/PushButton.h>
#include <motif/dialogs/MessageBox.h>
#include <motif/dialogs/FileSelectionBox.h>

#include <iostream>

/// Demonstrates dialog usage: MessageBox and FileSelectionBox.
int main() {
    using namespace motif;

    auto& app = Application::instance();
    if (!app.open()) return 1;

    Shell toplevel;
    toplevel.setTitle("Dialog Examples");
    toplevel.resize(300, 200);

    RowColumn layout(&toplevel);
    layout.setOrientation(RowColumn::Orientation::Vertical);
    layout.setSpacing(8);
    layout.setMargin(12, 12);

    // --- Info dialog ---
    PushButton infoBtn(&layout);
    infoBtn.setText("Show Info Message");
    infoBtn.addCallback(PushButton::ActivateCallback,
        [&](Widget&, void*) {
            auto dlg = std::make_unique<MessageBox>(&toplevel);
            dlg->setTitle("Information");
            dlg->setMessage("Operation completed successfully.");
            dlg->setType(MessageBox::Type::Information);
            dlg->setShowCancel(false);
            dlg->setShowHelp(false);
            dlg->setOkCallback([&dlg] { dlg->popdown(); });
            dlg->popup();
        });

    // --- Question dialog ---
    PushButton questionBtn(&layout);
    questionBtn.setText("Show Question");
    questionBtn.addCallback(PushButton::ActivateCallback,
        [&](Widget&, void*) {
            auto dlg = std::make_unique<MessageBox>(&toplevel);
            dlg->setTitle("Confirm");
            dlg->setMessage("Do you want to save changes?");
            dlg->setType(MessageBox::Type::Question);
            dlg->setOkLabel("Save");
            dlg->setCancelLabel("Don't Save");
            dlg->setOkCallback([] { std::cout << "Save clicked\n"; });
            dlg->setCancelCallback([] { std::cout << "Don't Save clicked\n"; });
            dlg->popup();
        });

    // --- Error dialog ---
    PushButton errorBtn(&layout);
    errorBtn.setText("Show Error");
    errorBtn.addCallback(PushButton::ActivateCallback,
        [&](Widget&, void*) {
            auto dlg = std::make_unique<MessageBox>(&toplevel);
            dlg->setTitle("Error");
            dlg->setMessage("File not found: /tmp/missing.txt");
            dlg->setType(MessageBox::Type::Error);
            dlg->setShowCancel(false);
            dlg->popup();
        });

    // --- File selection ---
    PushButton fileBtn(&layout);
    fileBtn.setText("Open File...");
    fileBtn.addCallback(PushButton::ActivateCallback,
        [&](Widget&, void*) {
            auto dlg = std::make_unique<FileSelectionBox>(&toplevel);
            dlg->setTitle("Select File");
            dlg->setPattern("*");
            dlg->setOkCallback([&dlg] {
                std::cout << "Selected: " << dlg->selectedFile() << "\n";
                dlg->popdown();
            });
            dlg->setCancelCallback([&dlg] { dlg->popdown(); });
            dlg->popup();
        });

    // --- Quit ---
    PushButton quitBtn(&layout);
    quitBtn.setText("Quit");
    quitBtn.addCallback(PushButton::ActivateCallback,
        [](Widget&, void*) { Application::instance().quit(); });

    toplevel.realize();
    app.run();
    return 0;
}
