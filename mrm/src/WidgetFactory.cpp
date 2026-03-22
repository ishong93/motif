#include <motif/mrm/ResourceLoader.h>
#include <motif/core/Widget.h>
#include <motif/core/Shell.h>
#include <motif/core/Manager.h>
#include <motif/core/Primitive.h>

// Widgets
#include <motif/widgets/Label.h>
#include <motif/widgets/PushButton.h>
#include <motif/widgets/ToggleButton.h>
#include <motif/widgets/ArrowButton.h>
#include <motif/widgets/CascadeButton.h>
#include <motif/widgets/DrawnButton.h>
#include <motif/widgets/Text.h>
#include <motif/widgets/TextField.h>
#include <motif/widgets/List.h>
#include <motif/widgets/Scale.h>
#include <motif/widgets/ScrollBar.h>
#include <motif/widgets/Separator.h>
#include <motif/widgets/ComboBox.h>

// Containers
#include <motif/containers/BulletinBoard.h>
#include <motif/containers/Form.h>
#include <motif/containers/RowColumn.h>
#include <motif/containers/Frame.h>
#include <motif/containers/PanedWindow.h>
#include <motif/containers/ScrolledWindow.h>
#include <motif/containers/MainWindow.h>
#include <motif/containers/Notebook.h>
#include <motif/containers/Container.h>
#include <motif/containers/TabStack.h>

// Dialogs
#include <motif/dialogs/DialogShell.h>
#include <motif/dialogs/MessageBox.h>
#include <motif/dialogs/SelectionBox.h>
#include <motif/dialogs/FileSelectionBox.h>
#include <motif/dialogs/Command.h>

// Menus
#include <motif/menus/MenuShell.h>
#include <motif/menus/MenuBar.h>
#include <motif/menus/PopupMenu.h>

namespace motif::mrm {

/// Helper: register a widget factory that creates T(parent) and sets its name
template <typename T>
static void reg(const std::string& className) {
    ResourceLoader::instance().registerWidgetClass(className,
        [](Widget* parent, const std::string& name) -> Widget* {
            auto* w = new T(parent);
            w->setName(name);
            return w;
        });
}

void registerAllBuiltinWidgets() {
    // Core
    reg<Shell>("XmDialogShell");

    // Primitives
    reg<Label>("XmLabel");
    reg<PushButton>("XmPushButton");
    reg<ToggleButton>("XmToggleButton");
    reg<ArrowButton>("XmArrowButton");
    reg<CascadeButton>("XmCascadeButton");
    reg<DrawnButton>("XmDrawnButton");
    reg<Text>("XmText");
    reg<TextField>("XmTextField");
    reg<List>("XmList");
    reg<Scale>("XmScale");
    reg<ScrollBar>("XmScrollBar");
    reg<Separator>("XmSeparator");
    reg<ComboBox>("XmComboBox");

    // Containers
    reg<BulletinBoard>("XmBulletinBoard");
    reg<Form>("XmForm");
    reg<RowColumn>("XmRowColumn");
    reg<Frame>("XmFrame");
    reg<PanedWindow>("XmPanedWindow");
    reg<ScrolledWindow>("XmScrolledWindow");
    reg<MainWindow>("XmMainWindow");
    reg<Notebook>("XmNotebook");
    reg<Container>("XmContainer");
    reg<TabStack>("XmTabStack");

    // Dialogs
    reg<DialogShell>("XmDialogShell");
    reg<MessageBox>("XmMessageBox");
    reg<SelectionBox>("XmSelectionBox");
    reg<FileSelectionBox>("XmFileSelectionBox");
    reg<Command>("XmCommand");

    // Menus
    reg<MenuShell>("XmMenuShell");
    reg<MenuBar>("XmMenuBar");
    reg<PopupMenu>("XmPopupMenu");

    // Gadgets (RowColumn as menu pane, etc.)
    reg<RowColumn>("XmCreatePulldownMenu");
    reg<RowColumn>("XmCreatePopupMenu");

    // Legacy aliases
    reg<Label>("XmLabelGadget");
    reg<PushButton>("XmPushButtonGadget");
    reg<ToggleButton>("XmToggleButtonGadget");
    reg<CascadeButton>("XmCascadeButtonGadget");
    reg<Separator>("XmSeparatorGadget");
    reg<ArrowButton>("XmArrowButtonGadget");
}

} // namespace motif::mrm
