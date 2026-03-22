# ── motif-wm: Motif Window Manager ───────────────────────────

add_executable(motif-wm
    src/main.cpp
    src/WindowManager.cpp
    src/EventHandler.cpp
    src/Decoration.cpp
    src/KeyFocus.cpp
    src/Menu.cpp
    src/EWMH.cpp
    src/ConfigParser.cpp
    src/SessionManager.cpp
)

target_include_directories(motif-wm
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(motif-wm PRIVATE cxx_std_17)

find_package(X11 REQUIRED)

find_library(SM_LIBRARY SM)
find_library(ICE_LIBRARY ICE)

target_link_libraries(motif-wm
    PRIVATE
        X11::X11
        ${SM_LIBRARY}
        ${ICE_LIBRARY}
)

include(GNUInstallDirs)

install(TARGETS motif-wm
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
