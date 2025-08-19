message(STATUS "Installing Visera Core...")

add_compile_definitions (NOMINMAX)
add_compile_options     (/utf-8)

set(VISERA_CORE_ROOT_DIR     ${PROJECT_SOURCE_DIR})
set(VISERA_CORE_SOURCE_DIR   "${VISERA_CORE_ROOT_DIR}/Source")
set(VISERA_CORE_EXTERNAL_DIR "${VISERA_CORE_ROOT_DIR}/External")
set(VISERA_CORE_GLOBAL_DIR   "${VISERA_CORE_ROOT_DIR}/Global")
set(VISERA_CORE_SCRIPTS_DIR  "${VISERA_CORE_ROOT_DIR}/Scripts")

add_library(${VISERA_CORE})

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

include(install_libpng)
link_libpng(${VISERA_CORE})

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