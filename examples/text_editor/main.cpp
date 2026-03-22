#include <motif/core/Application.h>
#include <motif/core/Shell.h>
#include <motif/containers/Form.h>
#include <motif/containers/MainWindow.h>
#include <motif/menus/MenuBar.h>
#include <motif/widgets/CascadeButton.h>
#include <motif/widgets/PushButton.h>
#include <motif/widgets/Text.h>
#include <motif/widgets/Label.h>
#include <motif/containers/RowColumn.h>

#include <iostream>
#include <fstream>

/// A simple text editor demonstrating MainWindow layout,
/// multi-line Text widget, and menu-driven file operations.
int main() {
    using namespace motif;

    auto& app = Application::instance();
    if (!app.open()) return 1;

    Shell toplevel;
    toplevel.setTitle("Text Editor");
    toplevel.resize(600, 450);

    MainWindow mainWin(&toplevel);

    // --- Menu bar ---
    MenuBar menuBar(&mainWin);

    CascadeButton fileCascade(&menuBar);
    fileCascade.setText("File");

    RowColumn fileMenu(&fileCascade);
    fileMenu.setOrientation(RowColumn::Orientation::Vertical);

    PushButton saveItem(&fileMenu);
    saveItem.setText("Save to /tmp/motif_output.txt");

    PushButton clearItem(&fileMenu);
    clearItem.setText("Clear");

    PushButton quitItem(&fileMenu);
    quitItem.setText("Quit");
    quitItem.addCallback(PushButton::ActivateCallback,
        [](Widget&, void*) { Application::instance().quit(); });

    // --- Text area ---
    Form workArea(&mainWin);

    Text editor(&workArea);
    editor.setEditable(true);
    editor.setRows(20);
    editor.setColumns(60);
    editor.setWordWrap(true);
    editor.setValue("Welcome to the Motif C++ text editor.\n\n"
                    "This demonstrates the multi-line Text widget with\n"
                    "word wrap, cursor positioning, and basic editing.\n");

    // Status bar
    Label statusBar(&workArea);
    statusBar.setText("Ready");
    statusBar.setAlignment(Label::Alignment::Beginning);

    // Constraints: editor fills top, status bar at bottom
    Form::Constraint editorC;
    editorC.topAttachment    = Form::Attachment::Form;
    editorC.leftAttachment   = Form::Attachment::Form;
    editorC.rightAttachment  = Form::Attachment::Form;
    editorC.bottomAttachment = Form::Attachment::Widget;
    editorC.bottomWidget     = &statusBar;
    editorC.topOffset = 4;
    editorC.leftOffset = 4;
    editorC.rightOffset = 4;
    editorC.bottomOffset = 4;
    workArea.setConstraint(&editor, editorC);

    Form::Constraint statusC;
    statusC.bottomAttachment = Form::Attachment::Form;
    statusC.leftAttachment   = Form::Attachment::Form;
    statusC.rightAttachment  = Form::Attachment::Form;
    statusC.leftOffset = 4;
    statusC.bottomOffset = 2;
    workArea.setConstraint(&statusBar, statusC);

    // Wire up callbacks
    saveItem.addCallback(PushButton::ActivateCallback,
        [&](Widget&, void*) {
            std::ofstream out("/tmp/motif_output.txt");
            if (out) {
                out << editor.value();
                statusBar.setText("Saved to /tmp/motif_output.txt");
                std::cout << "Saved " << editor.value().size() << " bytes\n";
            } else {
                statusBar.setText("Error: could not save file");
            }
        });

    clearItem.addCallback(PushButton::ActivateCallback,
        [&](Widget&, void*) {
            editor.setValue("");
            statusBar.setText("Cleared");
        });

    editor.addCallback(Text::ValueChangedCallback,
        [&](Widget&, void*) {
            int len = static_cast<int>(editor.value().size());
            statusBar.setText("Length: " + std::to_string(len) + " chars");
        });

    toplevel.realize();
    app.run();
    return 0;
}
