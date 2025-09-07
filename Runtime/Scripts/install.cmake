message(STATUS "\nInstalling Visera Runtime...")

set(VISERA_RUNTIME_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source")
set(VISERA_RUNTIME_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External")
set(VISERA_RUNTIME_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global")
set(VISERA_RUNTIME_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts")

add_library(${VISERA_RUNTIME} SHARED)
add_library(Visera::Runtime ALIAS ${VISERA_RUNTIME})
target_compile_definitions(${VISERA_RUNTIME} PRIVATE VISERA_RUNTIME_BUILD_SHARED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:${VISERA_APP}>")
add_custom_command(
        TARGET Visera::Runtime
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
        copy_if_different
        $<TARGET_FILE:Visera::Runtime>
        $<TARGET_FILE_DIR:${VISERA_APP}>)
if(MSVC)
    add_custom_command(
            TARGET Visera::Runtime
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E $<IF:$<BOOL:$<TARGET_PDB_FILE:Visera::Runtime>>,
            copy_if_different
            $<TARGET_PDB_FILE:Visera::Runtime>
            $<TARGET_FILE_DIR:${VISERA_APP}>)
endif()

if(NOT TARGET Visera::Core)
    message(FATAL_ERROR "Visera-Core is not installed!")
endif()
target_link_libraries(${VISERA_RUNTIME} PRIVATE Visera::Core)

#
# << Install External Packages >>
#

list(APPEND CMAKE_MODULE_PATH ${VISERA_RUNTIME_SCRIPTS_DIR})

include(install_dotnet)
link_dotnet(${VISERA_RUNTIME})

include(install_glfw)
link_glfw(${VISERA_RUNTIME})

include(install_vma)
link_vma(${VISERA_RUNTIME})

include(install_vulkan)
link_vulkan(${VISERA_RUNTIME})

include(install_volk)
link_volk(${VISERA_RUNTIME})

include(install_slang)
link_slang(${VISERA_RUNTIME})

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