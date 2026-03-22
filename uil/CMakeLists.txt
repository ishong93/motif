# ── motif-uil: UIL Compiler ───────────────────────────────────

# Library target (for tests and embedding)
add_library(motif-uil-lib STATIC
    src/Compiler.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/SymbolTable.cpp
)

target_include_directories(motif-uil-lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(motif-uil-lib
    PUBLIC motif
    PRIVATE motif-mrm
)

target_compile_features(motif-uil-lib PUBLIC cxx_std_17)

# Executable
add_executable(motif-uil src/main.cpp)

target_link_libraries(motif-uil
    PRIVATE motif-uil-lib motif-mrm
)

target_compile_features(motif-uil PRIVATE cxx_std_17)

include(GNUInstallDirs)

install(TARGETS motif-uil
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/motif/uil
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/motif
    FILES_MATCHING PATTERN "*.h"
)
