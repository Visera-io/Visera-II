if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_vulkan in_target)
    message(STATUS "\nLinking Vulkan (Vulkan::Vulkan)")

    if(NOT TARGET Vulkan::Vulkan)
        set(VULKAN_REQUIRED_MINIMAL_VERSION "1.4.0")

        if(NOT DEFINED ENV{VULKAN_SDK})
            if(APPLE)
                # Try to locate Vulkan SDK on macOS (default install location)
                file(GLOB VULKAN_SDK_CANDIDATES "/Users/*/VulkanSDK/*/macOS")
                list(SORT VULKAN_SDK_CANDIDATES COMPARE NATURAL)
                list(REVERSE VULKAN_SDK_CANDIDATES) # prefer latest
                list(GET VULKAN_SDK_CANDIDATES 0 VULKAN_SDK_PATH)

                if(EXISTS "${VULKAN_SDK_PATH}")
                    set(ENV{VULKAN_SDK} ${VULKAN_SDK_PATH})
                endif()
            endif(APPLE)

            message(FATAL_ERROR "Failed to find VulkanSDK on your system! Please set VULKAN_SDK.")
        endif()

        # Extract the version from the path
        string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" VULKAN_VERSION $ENV{VULKAN_SDK})

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
        endif(APPLE)
    endif()

    find_package(Vulkan REQUIRED)
    target_link_libraries(${in_target} PUBLIC Vulkan::Vulkan)
    target_include_directories(${in_target} BEFORE  PRIVATE ${VISERA_RUNTIME_EXTERNAL_DIR}/Vulkan/Include)
    target_compile_definitions(${in_target} PRIVATE VULKAN_HPP_NO_EXCEPTIONS)
    target_compile_definitions(${in_target} PRIVATE VULKAN_HPP_RAII_NO_EXCEPTIONS)
    target_compile_definitions(${in_target} PRIVATE VISERA_VULKAN_SDK_PATH="$ENV{VULKAN_SDK}")
endmacro()