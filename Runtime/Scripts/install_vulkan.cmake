if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_vulkan in_target)
    message(STATUS "\nLinking Vulkan (VulkanModule)")
    set(VULKAN_REQUIRED_MINIMAL_VERSION "1.4.328.0")
    set(VULKAN_DIR "${VISERA_RUNTIME_EXTERNAL_DIR}/Vulkan")

    if(NOT TARGET VulkanModule)
        add_subdirectory(${VULKAN_DIR})
        set_target_properties(VulkanModule PROPERTIES FOLDER "Visera/Runtime/External/Vulkan")
    endif()

    target_link_libraries(${in_target} PUBLIC VulkanModule)
    if(APPLE)
    target_link_libraries(${in_target} PUBLIC VulkanLoader)
    endif()

    message(STATUS "Vulkan files will be copied into the app bundle.")
    if(APPLE)
        add_custom_command(TARGET ${in_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Frameworks"
            "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Resources/Vulkan"
            # Vulkan
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VULKAN_DIR}/MacOS/libvulkan.1.dylib" # Note: this is a renamed lib
            "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Frameworks/"
            # MoltenVk
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VULKAN_DIR}/MacOS/libMoltenVK.dylib"
            "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Frameworks/"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VULKAN_DIR}/MacOS/MoltenVK_icd.json"
            "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Resources/Vulkan/"
            # Synchronization2
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VULKAN_DIR}/MacOS/libVkLayer_khronos_synchronization2.dylib"
            "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Frameworks/"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VULKAN_DIR}/MacOS/VkLayer_khronos_synchronization2.json"
            "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Resources/Vulkan/"
        )
        if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        add_custom_command(TARGET ${in_target} POST_BUILD
            # Validation Layer
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VULKAN_DIR}/MacOS/libVkLayer_khronos_validation.dylib"
            "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Frameworks/"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VULKAN_DIR}/MacOS/VkLayer_khronos_validation.json"
            "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Resources/Vulkan/"
        )
        endif()
    endif()
endmacro()