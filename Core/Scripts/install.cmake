message(STATUS "\nInstalling Visera Core...")

set(VISERA_CORE_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source")
set(VISERA_CORE_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External")
set(VISERA_CORE_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global")
set(VISERA_CORE_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts")

if(BuildSharedViseraCore)
    add_library(${VISERA_CORE} SHARED)
    target_compile_definitions(${VISERA_CORE} PRIVATE VISERA_CORE_BUILD_SHARED)
else()
    add_library(${VISERA_CORE} STATIC)
    target_compile_definitions(${VISERA_CORE} PRIVATE VISERA_CORE_BUILD_STATIC)
endif()
add_library(Visera::Core ALIAS ${VISERA_CORE})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:${VISERA_APP}>")
add_custom_command(
    TARGET Visera::Core
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:Visera::Core>
    $<TARGET_FILE_DIR:${VISERA_APP}>)
if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
add_custom_command(
    TARGET Visera::Core
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_PDB_FILE:Visera::Core>"
    "$<TARGET_FILE_DIR:${VISERA_APP}>"
)
endif()
#
# << Install External Packages >>
#
list(APPEND CMAKE_MODULE_PATH ${VISERA_CORE_SCRIPTS_DIR})

include(install_spdlog)
link_spdlog(${VISERA_CORE})

include(install_json)
link_json(${VISERA_CORE})

include(install_eigen)
link_eigen(${VISERA_CORE})

include(install_zlib)
link_zlib(${VISERA_CORE})

include(install_stb)
link_stb(${VISERA_CORE})

include(install_ankerl)
link_ankerl(${VISERA_CORE})

include(install_onetbb)
link_onetbb(${VISERA_CORE})

#
#
#
file(GLOB_RECURSE VISERA_CORE_MODULES "${VISERA_CORE_SOURCE_DIR}/*.ixx")

target_include_directories(${VISERA_CORE}
                           PUBLIC
                           ${VISERA_CORE_GLOBAL_DIR})

target_sources(${VISERA_CORE}
            PUBLIC
            FILE_SET "visera_core_modules" TYPE CXX_MODULES
            FILES ${VISERA_CORE_MODULES})

set_target_properties(${VISERA_CORE} PROPERTIES FOLDER "Visera/Core")