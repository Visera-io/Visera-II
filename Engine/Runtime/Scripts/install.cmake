# Defines install_visera_runtime(in_target), same pattern as install_visera_core / install_visera_platform.
# Must be included after all Runtime submodules have been add_subdirectory'd (so their install_visera_* exist).

macro(install_visera_runtime in_target)
    message(STATUS "\nInstalling Visera Runtime (all modules)...")
    install_visera_global   (${in_target})
    install_visera_tasks    (${in_target})
    install_visera_assethub (${in_target})
    install_visera_rhi      (${in_target})
    install_visera_shader   (${in_target})
    install_visera_graphics (${in_target})
    install_visera_audio    (${in_target})
    install_visera_input    (${in_target})
    install_visera_window   (${in_target})
endmacro()
