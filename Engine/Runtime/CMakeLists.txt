# Runtime root: unified paths and subdirectory for all Runtime modules.
# Included via Engine/CMakeLists.txt add_subdirectory(Runtime).
#
# VISERA_RUNTIME_API: defined in Global/Include/Visera-Global.hpp from CMake-set macros (PRIVATE, not propagated to App).
# VISERA_MONOLITHIC_MODE: set by Engine when building the monolithic lib.

set(VISERA_RUNTIME_DIR             "${CMAKE_CURRENT_SOURCE_DIR}")
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
        FILE_SET "visera_runtime_modules" TYPE CXX_MODULES
        FILES "${VISERA_RUNTIME_DIR}/Visera-Runtime.ixx")
endmacro()

# install_visera_runtime(${VISERA}) is called by Engine/CMakeLists.txt, same as install_visera_core / install_visera_platform.
