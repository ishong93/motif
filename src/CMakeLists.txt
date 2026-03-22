# ── libmotif: Core Widget Library ────────────────────────────

set(MOTIF_CORE_SOURCES
    core/Application.cpp
    core/Widget.cpp
    core/Primitive.cpp
    core/Manager.cpp
    core/Gadget.cpp
    core/Shell.cpp
    core/Resource.cpp
    core/CompoundString.cpp
    core/TypeRegistry.cpp
    core/Log.cpp
    core/Error.cpp
    core/FontRenderer.cpp
    core/Theme.cpp
    core/InputMethod.cpp
    core/HiDPI.cpp
    core/Accessibility.cpp
)

set(MOTIF_WIDGETS_SOURCES
    widgets/Label.cpp
    widgets/PushButton.cpp
    widgets/ToggleButton.cpp
    widgets/ArrowButton.cpp
    widgets/CascadeButton.cpp
    widgets/DrawnButton.cpp
    widgets/Text.cpp
    widgets/TextField.cpp
    widgets/List.cpp
    widgets/ScrollBar.cpp
    widgets/Scale.cpp
    widgets/Separator.cpp
    widgets/ComboBox.cpp
)

set(MOTIF_CONTAINERS_SOURCES
    containers/BulletinBoard.cpp
    containers/Form.cpp
    containers/RowColumn.cpp
    containers/Frame.cpp
    containers/PanedWindow.cpp
    containers/ScrolledWindow.cpp
    containers/MainWindow.cpp
    containers/Notebook.cpp
    containers/Container.cpp
    containers/TabStack.cpp
)

set(MOTIF_DIALOGS_SOURCES
    dialogs/DialogShell.cpp
    dialogs/MessageBox.cpp
    dialogs/SelectionBox.cpp
    dialogs/FileSelectionBox.cpp
    dialogs/Command.cpp
)

set(MOTIF_MENUS_SOURCES
    menus/MenuShell.cpp
    menus/MenuBar.cpp
    menus/PopupMenu.cpp
)

set(MOTIF_DND_SOURCES
    dnd/DragContext.cpp
    dnd/DragIcon.cpp
    dnd/DropSiteManager.cpp
    dnd/DropTransfer.cpp
)

set(MOTIF_GRAPHICS_SOURCES
    graphics/Color.cpp
    graphics/Draw.cpp
    graphics/ImageCache.cpp
    graphics/Pixmap.cpp
)

set(MOTIF_INPUT_SOURCES
    input/KeyBinding.cpp
    input/VirtualKey.cpp
    input/Traversal.cpp
)

set(MOTIF_CLIPBOARD_SOURCES
    clipboard/CutPaste.cpp
    clipboard/Transfer.cpp
)

set(MOTIF_PRINT_SOURCES
    print/PrintContext.cpp
)

set(MOTIF_MRM_SOURCES
    mrm/ResourceLoader.cpp
)

# ── Library Target ───────────────────────────────────────────
add_library(motif
    ${MOTIF_CORE_SOURCES}
    ${MOTIF_WIDGETS_SOURCES}
    ${MOTIF_CONTAINERS_SOURCES}
    ${MOTIF_DIALOGS_SOURCES}
    ${MOTIF_MENUS_SOURCES}
    ${MOTIF_DND_SOURCES}
    ${MOTIF_GRAPHICS_SOURCES}
    ${MOTIF_INPUT_SOURCES}
    ${MOTIF_CLIPBOARD_SOURCES}
    ${MOTIF_PRINT_SOURCES}
    ${MOTIF_MRM_SOURCES}
)

target_include_directories(motif
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

find_library(XPM_LIBRARY Xpm)

target_link_libraries(motif
    PUBLIC
        X11::X11
    PRIVATE
        ${XPM_LIBRARY}
)

# Xft/Fontconfig support
if(HAVE_XFT)
    target_compile_definitions(motif PUBLIC HAVE_XFT=1)
    target_include_directories(motif PRIVATE ${XFT_INCLUDE_DIRS} ${FONTCONFIG_INCLUDE_DIRS})
    target_link_libraries(motif PRIVATE ${XFT_LIBRARIES} ${FONTCONFIG_LIBRARIES})
endif()

# Cairo print support
if(HAVE_CAIRO)
    target_compile_definitions(motif PUBLIC HAVE_CAIRO=1)
    target_include_directories(motif PRIVATE ${CAIRO_INCLUDE_DIRS})
    target_link_libraries(motif PRIVATE ${CAIRO_LIBRARIES})
endif()

set_target_properties(motif PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME motif
)

# ── Install ──────────────────────────────────────────────────
include(GNUInstallDirs)

install(TARGETS motif
    EXPORT MotifTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
