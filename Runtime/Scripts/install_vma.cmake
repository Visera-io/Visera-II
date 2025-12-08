if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_vma in_target)
    message(STATUS "\nLinking VMA (GPUOpen::VulkanMemoryAllocator)")

    if(NOT TARGET GPUOpen::VulkanMemoryAllocator)
        add_subdirectory(${VISERA_RUNTIME_EXTERNAL_DIR}/VMA)
        target_compile_definitions(VulkanMemoryAllocator INTERFACE
            VMA_STATIC_VULKAN_FUNCTIONS=0
            VMA_DYNAMIC_VULKAN_FUNCTIONS=1)
        set_target_properties(VulkanMemoryAllocator PROPERTIES FOLDER "Visera/Runtime/External/VMA")
    endif()

    target_link_libraries(${in_target} PRIVATE GPUOpen::VulkanMemoryAllocator)
endmacro()