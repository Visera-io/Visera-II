if(NOT VISERA_TASKS_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_taskflow in_target)
    message(STATUS "\nLinking Taskflow (Taskflow::Taskflow)")

    if(NOT TARGET Taskflow::Taskflow)
        set(TF_BUILD_TESTS    OFF CACHE BOOL "")
        set(TF_BUILD_EXAMPLES OFF CACHE BOOL "")
        add_subdirectory(${VISERA_TASKS_EXTERNAL_DIR}/Taskflow)
        set_target_properties(Taskflow PROPERTIES FOLDER "Visera/Tasks/External/Taskflow")
    endif()

    target_link_libraries(${in_target} PRIVATE Taskflow::Taskflow)
endmacro()