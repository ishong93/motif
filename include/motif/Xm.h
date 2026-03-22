#pragma once

/// @file motif/Xm.h
/// Convenience header — includes all public Motif C++ headers.
/// For finer-grained control, include individual category headers instead.

#include <motif/Version.h>

// ── Core ────────────────────────────────────────────────────
#include <motif/core/Application.h>
#include <motif/core/Widget.h>
#include <motif/core/Primitive.h>
#include <motif/core/Manager.h>
#include <motif/core/Gadget.h>
#include <motif/core/Shell.h>
#include <motif/core/Resource.h>
#include <motif/core/CompoundString.h>
#include <motif/core/Log.h>
#include <motif/core/Error.h>
#include <motif/core/FontRenderer.h>
#include <motif/core/Theme.h>
#include <motif/core/InputMethod.h>
#include <motif/core/TypeRegistry.h>

// ── Widgets ─────────────────────────────────────────────────
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
#include <motif/widgets/ComboBox.h>
#include <motif/widgets/Separator.h>

// ── Containers ──────────────────────────────────────────────
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

// ── Dialogs ─────────────────────────────────────────────────
#include <motif/dialogs/DialogShell.h>
#include <motif/dialogs/MessageBox.h>
#include <motif/dialogs/SelectionBox.h>
#include <motif/dialogs/FileSelectionBox.h>
#include <motif/dialogs/Command.h>

// ── Menus ───────────────────────────────────────────────────
#include <motif/menus/MenuShell.h>
#include <motif/menus/MenuBar.h>
#include <motif/menus/PopupMenu.h>

// ── Graphics ────────────────────────────────────────────────
#include <motif/graphics/Color.h>
#include <motif/graphics/Draw.h>
#include <motif/graphics/ImageCache.h>
#include <motif/graphics/Pixmap.h>

// ── Input ───────────────────────────────────────────────────
#include <motif/input/KeyBinding.h>
#include <motif/input/VirtualKey.h>
#include <motif/input/Traversal.h>

// ── Clipboard ───────────────────────────────────────────────
#include <motif/clipboard/CutPaste.h>
#include <motif/clipboard/Transfer.h>

// ── Drag and Drop ───────────────────────────────────────────
#include <motif/dnd/DragContext.h>
#include <motif/dnd/DragIcon.h>
#include <motif/dnd/DropSiteManager.h>
#include <motif/dnd/DropTransfer.h>

// ── MRM (Resource Manager) ──────────────────────────────────
#include <motif/mrm/ResourceLoader.h>
