cmake_minimum_required(VERSION 3.20)

project(Motif
    VERSION 3.0.0
    DESCRIPTION "Motif GUI Toolkit - C++ Rewrite"
    LANGUAGES CXX
)

# ── C++ Standard ─────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Build Options ────────────────────────────────────────────
option(MOTIF_BUILD_TESTS    "Build unit tests"          ON)
option(MOTIF_BUILD_EXAMPLES "Build example programs"    ON)
option(MOTIF_BUILD_MRM      "Build resource manager"    ON)
option(MOTIF_BUILD_UIL      "Build UIL compiler"        ON)
option(MOTIF_BUILD_WM       "Build window manager"      ON)

# ── Output Directories ──────────────────────────────────────
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ── Dependencies ─────────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(X11 REQUIRED)

# Xft/Fontconfig for anti-aliased font rendering (optional)
option(MOTIF_USE_XFT "Enable Xft anti-aliased font rendering" ON)
if(MOTIF_USE_XFT)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(XFT xft)
        pkg_check_modules(FONTCONFIG fontconfig)
    endif()
    if(XFT_FOUND AND FONTCONFIG_FOUND)
        set(HAVE_XFT TRUE)
        message(STATUS "Xft font rendering: enabled")
    else()
        set(HAVE_XFT FALSE)
        message(STATUS "Xft font rendering: disabled (Xft/Fontconfig not found)")
    endif()
else()
    set(HAVE_XFT FALSE)
    message(STATUS "Xft font rendering: disabled (MOTIF_USE_XFT=OFF)")
endif()

# Cairo for print/PDF output (optional)
option(MOTIF_USE_CAIRO "Enable Cairo PDF/PostScript print output" ON)
if(MOTIF_USE_CAIRO)
    if(NOT PkgConfig_FOUND)
        find_package(PkgConfig)
    endif()
    if(PkgConfig_FOUND)
        pkg_check_modules(CAIRO cairo cairo-pdf cairo-ps)
    endif()
    if(CAIRO_FOUND)
        set(HAVE_CAIRO TRUE)
        message(STATUS "Cairo print support: enabled")
    else()
        set(HAVE_CAIRO FALSE)
        message(STATUS "Cairo print support: disabled (cairo not found)")
    endif()
else()
    set(HAVE_CAIRO FALSE)
    message(STATUS "Cairo print support: disabled (MOTIF_USE_CAIRO=OFF)")
endif()

# ── Core Library ─────────────────────────────────────────────
add_subdirectory(src)

# ── Sub-projects ─────────────────────────────────────────────
if(MOTIF_BUILD_MRM)
    add_subdirectory(mrm)
endif()

if(MOTIF_BUILD_UIL)
    add_subdirectory(uil)
endif()

if(MOTIF_BUILD_WM)
    add_subdirectory(wm)
endif()

# ── Tests ────────────────────────────────────────────────────
if(MOTIF_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ── Examples ─────────────────────────────────────────────────
if(MOTIF_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ── Install ──────────────────────────────────────────────────
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(DIRECTORY include/motif
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY resources/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/motif
)

install(DIRECTORY bindings/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/motif/bindings
    PATTERN "Makefile*" EXCLUDE
)

# ── CMake Config Export ──────────────────────────────────────
install(EXPORT MotifTargets
    FILE MotifTargets.cmake
    NAMESPACE Motif::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Motif
)

configure_package_config_file(
    cmake/MotifConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/MotifConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Motif
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/MotifConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MotifConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MotifConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Motif
)

# ── pkg-config ───────────────────────────────────────────────
configure_file(
    cmake/motif.pc.in
    "${CMAKE_CURRENT_BINARY_DIR}/motif.pc"
    @ONLY
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/motif.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
