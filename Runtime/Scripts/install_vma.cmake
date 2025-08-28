if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_vma in_target)
    message(STATUS "\nLinking VMA (GPUOpen::VulkanMemoryAllocator)")

    if(NOT TARGET GPUOpen::VulkanMemoryAllocator)
        add_subdirectory(${VISERA_RUNTIME_EXTERNAL_DIR}/VMA)
    endif()

    target_link_libraries(${in_target} PUBLIC GPUOpen::VulkanMemoryAllocator)
endmacro()