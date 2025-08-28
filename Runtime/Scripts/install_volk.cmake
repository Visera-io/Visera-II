if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_volk in_target)
    message(STATUS "\nLinking Volk (volk::volk)")

    if(NOT TARGET volk::volk)
        find_package(Vulkan REQUIRED)
        add_subdirectory(${VISERA_RUNTIME_EXTERNAL_DIR}/Volk)
    endif()

    target_link_libraries(${in_target} PUBLIC volk::volk)
endmacro()