if(NOT VISERA_RHI_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_vulkan in_target)
    message(STATUS "\nLinking Vulkan (VulkanModule)")

    if(NOT TARGET VulkanModule)
        add_subdirectory("${VISERA_RHI_EXTERNAL_DIR}/Vulkan")
        set_target_properties(VulkanModule PROPERTIES FOLDER "Visera/Runtime/External/Vulkan")
        set_target_properties(VulkanLoader PROPERTIES FOLDER "Visera/Runtime/External/Vulkan")
    endif()

    target_link_libraries(${in_target} PUBLIC VulkanModule)
endmacro()