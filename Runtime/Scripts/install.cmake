message(STATUS "\nInstalling Visera Runtime...")

set(VISERA_RUNTIME_ROOT_DIR     ${PROJECT_SOURCE_DIR})
set(VISERA_RUNTIME_SOURCE_DIR   "${VISERA_RUNTIME_ROOT_DIR}/Source")
set(VISERA_RUNTIME_EXTERNAL_DIR "${VISERA_RUNTIME_ROOT_DIR}/External")
set(VISERA_RUNTIME_GLOBAL_DIR   "${VISERA_RUNTIME_ROOT_DIR}/Global")
set(VISERA_RUNTIME_SCRIPTS_DIR  "${VISERA_RUNTIME_ROOT_DIR}/Scripts")

add_library(${VISERA_RUNTIME})
add_library(Visera::Runtime ALIAS ${VISERA_RUNTIME})

if(NOT TARGET Visera::Core)
    message(FATAL_ERROR "Visera-Core is not installed!")
endif()
target_link_libraries(${VISERA_RUNTIME} PRIVATE Visera::Core)

#
# << Install External Packages >>
#

list(APPEND CMAKE_MODULE_PATH ${VISERA_RUNTIME_SCRIPTS_DIR})

include(install_glfw) #[TODO]: Editor/Studio
link_glfw(${VISERA_RUNTIME})

include(install_vma)
link_vma(${VISERA_RUNTIME})

#
#
#
file(GLOB_RECURSE VISERA_RUNTIME_MODULES "${VISERA_RUNTIME_SOURCE_DIR}/*.ixx")

target_include_directories(${VISERA_RUNTIME}
                           PUBLIC
                           ${VISERA_RUNTIME_GLOBAL_DIR})

target_sources(${VISERA_RUNTIME}
               PUBLIC
               FILE_SET "visera_runtime_modules" TYPE CXX_MODULES
               FILES ${VISERA_RUNTIME_MODULES})