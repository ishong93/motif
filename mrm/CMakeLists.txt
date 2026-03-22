# ── libmotif-mrm: Resource Manager ───────────────────────────

add_library(motif-mrm
    src/ResourceManager.cpp
    src/Hierarchy.cpp
    src/WidgetFactory.cpp
)

target_include_directories(motif-mrm
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(motif-mrm PRIVATE cxx_std_17)

target_link_libraries(motif-mrm
    PUBLIC motif
)

set_target_properties(motif-mrm PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

include(GNUInstallDirs)

install(TARGETS motif-mrm
    EXPORT MotifTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/motif/mrm
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/motif
    FILES_MATCHING PATTERN "*.h"
)
