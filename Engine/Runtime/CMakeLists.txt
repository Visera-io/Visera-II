# Runtime root: unified paths and subdirectory for all Runtime modules.
# Included via Engine/CMakeLists.txt add_subdirectory(Runtime).

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
add_subdirectory(${VISERA_RUNTIME_ASSETHUB_DIR})
add_subdirectory(${VISERA_RUNTIME_RHI_DIR})
add_subdirectory(${VISERA_RUNTIME_GRAPHICS_DIR})
add_subdirectory(${VISERA_RUNTIME_AUDIO_DIR})
add_subdirectory(${VISERA_RUNTIME_INPUT_DIR})
add_subdirectory(${VISERA_RUNTIME_WINDOW_DIR})

macro(install_visera_runtime in_target)
    message(STATUS "\nInstalling Visera Runtime (all modules)...")
    install_visera_global   (${in_target})
    install_visera_tasks    (${in_target})
    install_visera_assethub (${in_target})
    install_visera_rhi      (${in_target})
    install_visera_graphics (${in_target})
    install_visera_audio    (${in_target})
    install_visera_input    (${in_target})
    install_visera_window   (${in_target})
endmacro()

if(VISERA_MONOLITHIC_MODE)
    # install_visera_runtime(${VISERA}) is called by Engine/CMakeLists.txt, same as install_visera_core / install_visera_platform
else()
    # Umbrella target: link Visera::Runtime to get all Runtime modules
    add_library(Visera-Runtime INTERFACE)
    add_library(Visera::Runtime ALIAS Visera-Runtime)
    target_link_libraries(Visera-Runtime INTERFACE
        Visera::Global
        Visera::Tasks
        Visera::AssetHub
        Visera::RHI
        Visera::Graphics
        Visera::Audio
        Visera::Input
        Visera::Window
    )
endif()
