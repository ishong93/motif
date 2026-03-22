# ── Tests ────────────────────────────────────────────────────

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# ── Helper function ──────────────────────────────────────────
function(motif_add_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME}
        PRIVATE motif GTest::gtest_main
    )
    gtest_discover_tests(${TEST_NAME})
endfunction()

# ── Core Tests ───────────────────────────────────────────────
motif_add_test(test_widget          core/WidgetTest.cpp)
motif_add_test(test_resource        core/ResourceTest.cpp)
motif_add_test(test_gadget          core/GadgetTest.cpp)
motif_add_test(test_primitive       core/PrimitiveTest.cpp)
motif_add_test(test_manager         core/ManagerTest.cpp)
motif_add_test(test_compound_string core/CompoundStringTest.cpp)
motif_add_test(test_log             core/LogTest.cpp)

# ── Widget Tests ─────────────────────────────────────────────
motif_add_test(test_label           widgets/LabelTest.cpp)
motif_add_test(test_pushbutton      widgets/PushButtonTest.cpp)
motif_add_test(test_togglebutton    widgets/ToggleButtonTest.cpp)
motif_add_test(test_scale           widgets/ScaleTest.cpp)
motif_add_test(test_scrollbar       widgets/ScrollBarTest.cpp)
motif_add_test(test_text            widgets/TextTest.cpp)
motif_add_test(test_list            widgets/ListTest.cpp)
motif_add_test(test_combobox        widgets/ComboBoxTest.cpp)

# ── Container Tests ──────────────────────────────────────────
motif_add_test(test_rowcolumn       containers/RowColumnTest.cpp)
motif_add_test(test_form            containers/FormTest.cpp)
motif_add_test(test_frame           containers/FrameTest.cpp)
motif_add_test(test_panedwindow     containers/PanedWindowTest.cpp)
motif_add_test(test_notebook        containers/NotebookTest.cpp)
motif_add_test(test_tabstack        containers/TabStackTest.cpp)
motif_add_test(test_bulletinboard   containers/BulletinBoardTest.cpp)

# ── Dialog Tests ────────────────────────────────────────────
motif_add_test(test_messagebox      dialogs/MessageBoxTest.cpp)
motif_add_test(test_selectionbox    dialogs/SelectionBoxTest.cpp)
motif_add_test(test_fileselbox      dialogs/FileSelectionBoxTest.cpp)
motif_add_test(test_command         dialogs/CommandTest.cpp)

# ── DnD Tests ───────────────────────────────────────────────
motif_add_test(test_dragcontext     dnd/DragContextTest.cpp)
motif_add_test(test_dropsite        dnd/DropSiteManagerTest.cpp)

# ── MRM Tests ───────────────────────────────────────────────
motif_add_test(test_resourceloader  mrm/ResourceLoaderTest.cpp)

# ── Mock X11 Test Support Library ───────────────────────────
add_library(motif_test_mock STATIC mock/MockDisplay.cpp)
target_include_directories(motif_test_mock PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(motif_test_mock PUBLIC motif GTest::gtest)

# ── UIL Compiler Tests ──────────────────────────────────────
if(MOTIF_BUILD_UIL)
    add_executable(test_uil_compiler uil/CompilerTest.cpp)
    target_link_libraries(test_uil_compiler
        PRIVATE motif-uil-lib GTest::gtest_main
    )
    gtest_discover_tests(test_uil_compiler)
endif()
