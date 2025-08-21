message(STATUS "\nInstalling Visera Studio...")

set(VISERA_STUDIO_ROOT_DIR     ${PROJECT_SOURCE_DIR})
set(VISERA_STUDIO_SOURCE_DIR   "${VISERA_STUDIO_ROOT_DIR}/Source")
set(VISERA_STUDIO_EXTERNAL_DIR "${VISERA_STUDIO_ROOT_DIR}/External")
set(VISERA_STUDIO_GLOBAL_DIR   "${VISERA_STUDIO_ROOT_DIR}/Global")
set(VISERA_STUDIO_SCRIPTS_DIR  "${VISERA_STUDIO_ROOT_DIR}/Scripts")

add_library(${VISERA_STUDIO})
add_library(Visera::Studio ALIAS ${VISERA_STUDIO})

if(NOT TARGET Visera::Core)
    message(FATAL_ERROR "Visera-Core is not installed!")
endif()
target_link_libraries(${VISERA_STUDIO} PRIVATE Visera::Core)

if(NOT TARGET Visera::Runtime)
    message(FATAL_ERROR "Visera-Runtime is not installed!")
endif()
target_link_libraries(${VISERA_STUDIO} PRIVATE Visera::Runtime)

if(NOT TARGET Visera::Engine)
    message(FATAL_ERROR "Visera-Engine is not installed!")
endif()
target_link_libraries(${VISERA_STUDIO} PRIVATE Visera::Engine)

#
# << Install External Packages >>
#

list(APPEND CMAKE_MODULE_PATH ${VISERA_STUDIO_SCRIPTS_DIR})

#
#
#
file(GLOB_RECURSE VISERA_STUDIO_MODULES "${VISERA_STUDIO_SOURCE_DIR}/*.ixx")

target_include_directories(${VISERA_STUDIO}
                           PUBLIC
                           ${VISERA_STUDIO_GLOBAL_DIR})

target_sources(${VISERA_STUDIO}
               PUBLIC
               FILE_SET "visera_studio_modules" TYPE CXX_MODULES
               FILES ${VISERA_STUDIO_MODULES})