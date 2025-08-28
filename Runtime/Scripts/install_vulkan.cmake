if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_vulkan in_target)
    message(STATUS "\nLinking Vulkan (Vulkan::Vulkan)")

    if(NOT TARGET Vulkan::Vulkan)
        set(VULKAN_REQUIRED_MINIMAL_VERSION "1.4.0")

        if(NOT DEFINED ENV{VULKAN_SDK})
            message(FATAL_ERROR "Failed to find VulkanSDK on your system!")
        else()
            # Extract the version from the path
            string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" VULKAN_VERSION "$ENV{VULKAN_SDK}")

            if(VULKAN_VERSION)
                message(STATUS "Found VulkanSDK ${VULKAN_VERSION} on current system")
                # Compare with required version (1.4.0)
                if(VULKAN_VERSION VERSION_LESS ${VULKAN_REQUIRED_MINIMAL_VERSION})
                    message(FATAL_ERROR "VulkanSDK version is required >= 1.4.0, while current version is ${VULKAN_VERSION}")
                endif()
            else()
                message(FATAL_ERROR "Could not determine Vulkan SDK version.")
            endif()

            if(APPLE)
                execute_process(
                        COMMAND vulkaninfo --summary --text
                        OUTPUT_VARIABLE VULKANINFO_OUT
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                )

                # Extract the driverInfo line
                string(REGEX MATCH "driverInfo[ \t]*=[ \t]*([0-9]+\\.[0-9]+\\.[0-9]+)"
                        _ "${VULKANINFO_OUT}")
                set(MOLTENVK_VERSION "${CMAKE_MATCH_1}")

                if(MOLTENVK_VERSION STREQUAL "")
                    message(FATAL_ERROR "Could not detect MoltenVK runtime version from vulkaninfo output: ${VULKAN_DRIVER_INFO}")
                else()
                    message(STATUS "MoltenVK runtime version detected: ${MOLTENVK_VERSION}")

                    if (MOLTENVK_VERSION VERSION_LESS "1.3.0")
                        message(FATAL_ERROR "MoltenVK runtime version must be >= 1.3.0 (found ${MOLTENVK_VERSION})")
                    endif()
                endif()
            endif()
        endif()
    endif()

    find_package(Vulkan REQUIRED)
    target_link_libraries(${in_target} PUBLIC Vulkan::Vulkan)
endmacro()