if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please set VISERA_RUNTIME_EXTERNAL_DIR (e.g. from Runtime/CMakeLists.txt)")
endif()

macro(link_vulkan in_target)
    message(STATUS "\nLinking Vulkan (VulkanModule)")

    if(NOT TARGET VulkanModule)
        add_subdirectory("${VISERA_RUNTIME_EXTERNAL_DIR}/Vulkan")
        set_target_properties(VulkanModule PROPERTIES FOLDER "Visera/Runtime/External/Vulkan")
        set_target_properties(VulkanLoader PROPERTIES FOLDER "Visera/Runtime/External/Vulkan")
    endif()

    target_link_libraries(${in_target} PRIVATE "$<BUILD_INTERFACE:VulkanModule>")
endmacro()