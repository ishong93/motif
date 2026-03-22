#include <motif/core/Application.h>
#include <motif/core/Shell.h>
#include <motif/containers/MainWindow.h>
#include <motif/containers/RowColumn.h>
#include <motif/menus/MenuBar.h>
#include <motif/widgets/CascadeButton.h>
#include <motif/widgets/PushButton.h>
#include <motif/widgets/Label.h>
#include <motif/widgets/Separator.h>
#include <motif/menus/PopupMenu.h>

#include <iostream>

/// Demonstrates menu bar with cascading menus and popup context menu.
int main() {
    using namespace motif;

    auto& app = Application::instance();
    if (!app.open()) return 1;

    Shell toplevel;
    toplevel.setTitle("Menu Example");
    toplevel.resize(400, 300);

    MainWindow mainWin(&toplevel);

    // --- Menu bar ---
    MenuBar menuBar(&mainWin);

    // File menu
    CascadeButton fileCascade(&menuBar);
    fileCascade.setText("File");

    RowColumn fileMenu(&fileCascade);
    fileMenu.setOrientation(RowColumn::Orientation::Vertical);

    PushButton newItem(&fileMenu);
    newItem.setText("New");
    newItem.addCallback(PushButton::ActivateCallback,
        [](Widget&, void*) { std::cout << "File > New\n"; });

    PushButton openItem(&fileMenu);
    openItem.setText("Open...");
    openItem.addCallback(PushButton::ActivateCallback,
        [](Widget&, void*) { std::cout << "File > Open\n"; });

    Separator sep1(&fileMenu);

    PushButton quitItem(&fileMenu);
    quitItem.setText("Quit");
    quitItem.addCallback(PushButton::ActivateCallback,
        [](Widget&, void*) { Application::instance().quit(); });

    // Edit menu
    CascadeButton editCascade(&menuBar);
    editCascade.setText("Edit");

    RowColumn editMenu(&editCascade);
    editMenu.setOrientation(RowColumn::Orientation::Vertical);

    PushButton cutItem(&editMenu);
    cutItem.setText("Cut");
    cutItem.addCallback(PushButton::ActivateCallback,
        [](Widget&, void*) { std::cout << "Edit > Cut\n"; });

    PushButton copyItem(&editMenu);
    copyItem.setText("Copy");
    copyItem.addCallback(PushButton::ActivateCallback,
        [](Widget&, void*) { std::cout << "Edit > Copy\n"; });

    PushButton pasteItem(&editMenu);
    pasteItem.setText("Paste");
    pasteItem.addCallback(PushButton::ActivateCallback,
        [](Widget&, void*) { std::cout << "Edit > Paste\n"; });

    // Help menu (right-aligned)
    CascadeButton helpCascade(&menuBar);
    helpCascade.setText("Help");
    menuBar.setHelpWidget(&helpCascade);

    RowColumn helpMenu(&helpCascade);
    helpMenu.setOrientation(RowColumn::Orientation::Vertical);

    PushButton aboutItem(&helpMenu);
    aboutItem.setText("About...");
    aboutItem.addCallback(PushButton::ActivateCallback,
        [](Widget&, void*) { std::cout << "Help > About\n"; });

    // --- Work area ---
    RowColumn workArea(&mainWin);
    workArea.setOrientation(RowColumn::Orientation::Vertical);
    workArea.setMargin(12, 12);

    Label infoLabel(&workArea);
    infoLabel.setText("Right-click for context menu");
    infoLabel.setAlignment(Label::Alignment::Center);

    toplevel.realize();
    app.run();
    return 0;
}
