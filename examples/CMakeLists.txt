# ── Example Programs ─────────────────────────────────────────

function(motif_add_example EXAMPLE_NAME EXAMPLE_SOURCE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
    target_link_libraries(${EXAMPLE_NAME} PRIVATE motif)
endfunction()

motif_add_example(hello_motif     hello_motif/main.cpp)
motif_add_example(form_layout     form_layout/main.cpp)
motif_add_example(dialogs         dialogs/main.cpp)
motif_add_example(menus           menus/main.cpp)
motif_add_example(themes          themes/main.cpp)
motif_add_example(text_editor     text_editor/main.cpp)
motif_add_example(quickstart      quickstart/main.cpp)
