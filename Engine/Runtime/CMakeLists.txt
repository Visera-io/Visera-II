# Runtime root: unified paths and subdirectory for all Runtime modules.
# Included via Engine/CMakeLists.txt add_subdirectory(Runtime).
#
# VISERA_RUNTIME_API: defined in Global/Include/Visera-Global.hpp from CMake-set macros (PRIVATE, not propagated to App).
# - VISERA_RUNTIME_BUILD_SHARED: set only here for all non-monolithic Runtime targets (Global decides VISERA_RUNTIME_API in header).
# - VISERA_MONOLITHIC_MODE: set by Engine when building the monolithic lib.

set(VISERA_RUNTIME_DIR             "${CMAKE_CURRENT_SOURCE_DIR}")
set(VISERA_RUNTIME_SCRIPTS_DIR     "${VISERA_RUNTIME_DIR}/Scripts")
set(VISERA_RUNTIME_GLOBAL_DIR      "${VISERA_RUNTIME_DIR}/Global")
set(VISERA_RUNTIME_TASKS_DIR       "${VISERA_RUNTIME_DIR}/Tasks")
set(VISERA_RUNTIME_ASSETHUB_DIR    "${VISERA_RUNTIME_DIR}/AssetHub")
set(VISERA_RUNTIME_RHI_DIR         "${VISERA_RUNTIME_DIR}/RHI")
set(VISERA_RUNTIME_GRAPHICS_DIR    "${VISERA_RUNTIME_DIR}/Graphics")
set(VISERA_RUNTIME_AUDIO_DIR       "${VISERA_RUNTIME_DIR}/Audio")
set(VISERA_RUNTIME_INPUT_DIR       "${VISERA_RUNTIME_DIR}/Input")
set(VISERA_RUNTIME_WINDOW_DIR      "${VISERA_RUNTIME_DIR}/Window")

add_subdirectory(${VISERA_RUNTIME_GLOBAL_DIR})
add_subdirectory(${VISERA_RUNTIME_TASKS_DIR})
add_subdirectory(${VISERA_RUNTIME_WINDOW_DIR})
add_subdirectory(${VISERA_RUNTIME_RHI_DIR})
add_subdirectory(${VISERA_RUNTIME_GRAPHICS_DIR})
add_subdirectory(${VISERA_RUNTIME_AUDIO_DIR})
add_subdirectory(${VISERA_RUNTIME_ASSETHUB_DIR})
add_subdirectory(${VISERA_RUNTIME_INPUT_DIR})

# Non-monolithic: aliases and VISERA_RUNTIME_BUILD_SHARED for all Runtime DLLs (Global header decides VISERA_RUNTIME_API)
if(NOT VISERA_MONOLITHIC_MODE)
    add_library(Visera::Runtime::Global   ALIAS Visera-Global)
    add_library(Visera::Runtime::Tasks    ALIAS Visera-Tasks)
    add_library(Visera::Runtime::Window   ALIAS Visera-Window)
    add_library(Visera::Runtime::RHI      ALIAS Visera-RHI)
    add_library(Visera::Runtime::Graphics ALIAS Visera-Graphics)
    add_library(Visera::Runtime::Audio    ALIAS Visera-Audio)
    add_library(Visera::Runtime::AssetHub ALIAS Visera-AssetHub)
    add_library(Visera::Runtime::Input    ALIAS Visera-Input)
    target_compile_definitions(Visera-Global   PRIVATE VISERA_RUNTIME_BUILD_SHARED)
    target_compile_definitions(Visera-Tasks    PRIVATE VISERA_RUNTIME_BUILD_SHARED)
    target_compile_definitions(Visera-Window   PRIVATE VISERA_RUNTIME_BUILD_SHARED)
    target_compile_definitions(Visera-RHI      PRIVATE VISERA_RUNTIME_BUILD_SHARED)
    target_compile_definitions(Visera-Graphics PRIVATE VISERA_RUNTIME_BUILD_SHARED)
    target_compile_definitions(Visera-Audio    PRIVATE VISERA_RUNTIME_BUILD_SHARED)
    target_compile_definitions(Visera-AssetHub PRIVATE VISERA_RUNTIME_BUILD_SHARED)
    target_compile_definitions(Visera-Input    PRIVATE VISERA_RUNTIME_BUILD_SHARED)
endif()

# Wrapper macros: new-style names that forward to subdirectory install macros
macro(install_visera_runtime_global in_target)
    install_visera_global(${in_target})
endmacro()
macro(install_visera_runtime_tasks in_target)
    install_visera_tasks(${in_target})
endmacro()
macro(install_visera_runtime_window in_target)
    install_visera_window(${in_target})
endmacro()
macro(install_visera_runtime_rhi in_target)
    install_visera_rhi(${in_target})
endmacro()
macro(install_visera_runtime_graphics in_target)
    install_visera_graphics(${in_target})
endmacro()
macro(install_visera_runtime_audio in_target)
    install_visera_audio(${in_target})
endmacro()
macro(install_visera_runtime_assethub in_target)
    install_visera_assethub(${in_target})
endmacro()
macro(install_visera_runtime_input in_target)
    install_visera_input(${in_target})
endmacro()

macro(install_visera_runtime in_target)
    message(STATUS "\nInstalling Visera Runtime (all modules)...")
    install_visera_runtime_global   (${in_target})
    install_visera_runtime_tasks    (${in_target})
    install_visera_runtime_window   (${in_target})
    install_visera_runtime_rhi      (${in_target})
    install_visera_runtime_graphics (${in_target})
    install_visera_runtime_audio    (${in_target})
    install_visera_runtime_assethub (${in_target})
    install_visera_runtime_input    (${in_target})
    # Umbrella module Visera.Runtime (single import for apps)
    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_runtime_umbrella" TYPE CXX_MODULES
        FILES "${VISERA_RUNTIME_DIR}/Visera-Runtime.ixx")
endmacro()

if(VISERA_MONOLITHIC_MODE)
    # install_visera_runtime(${VISERA}) is called by Engine/CMakeLists.txt, same as install_visera_core / install_visera_platform
else()
    # Umbrella library: compiles Visera.Runtime module and re-exports all Runtime modules
    add_library(Visera-Runtime SHARED "${VISERA_RUNTIME_DIR}/Visera-Runtime.ixx")
    add_library(Visera::Runtime ALIAS Visera-Runtime)
    target_compile_definitions(Visera-Runtime PRIVATE VISERA_RUNTIME_BUILD_SHARED)
    set_target_properties(Visera-Runtime PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
    if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        add_custom_command(
            TARGET Visera-Runtime
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_PDB_FILE:Visera-Runtime>"
            "${VISERA_APP_FRAMEWORK_DIR}"
        )
    endif()
    target_link_libraries(Visera-Runtime PUBLIC
        Visera::Runtime::Global
        Visera::Runtime::Tasks
        Visera::Runtime::AssetHub
        Visera::Runtime::RHI
        Visera::Runtime::Graphics
        Visera::Runtime::Audio
        Visera::Runtime::Input
        Visera::Runtime::Window
    )
    set_target_properties(Visera-Runtime PROPERTIES FOLDER "Visera/Runtime")
endif()
