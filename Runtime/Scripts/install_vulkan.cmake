if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_vulkan in_target)
    message(STATUS "\nLinking Vulkan (Vulkan::Vulkan)")

    if(NOT TARGET Vulkan::Vulkan)
        if(APPLE)
            set(VULKAN_REQUIRED_MINIMAL_VERSION "1.4.0")
        else()
            set(VULKAN_REQUIRED_MINIMAL_VERSION "1.4.328") # MoltenVk 1.4.0
        endif()

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
        string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+" VULKAN_VERSION $ENV{VULKAN_SDK})

        if(VULKAN_VERSION)
            message(STATUS "Found VulkanSDK ${VULKAN_VERSION} on current system")

            if(VULKAN_VERSION VERSION_LESS ${VULKAN_REQUIRED_MINIMAL_VERSION})
                message(FATAL_ERROR "VulkanSDK version is required >= 1.4.0, while current version is ${VULKAN_VERSION}")
            endif()
        else()
            message(FATAL_ERROR "Could not determine Vulkan SDK version.")
        endif()
    endif()

    find_package(Vulkan REQUIRED)
    target_link_libraries(${in_target} PUBLIC Vulkan::Vulkan)

    target_compile_definitions(${in_target} PRIVATE
            VISERA_VULKAN_SDK_PATH="$ENV{VULKAN_SDK}"
            VULKAN_HPP_NO_EXCEPTIONS
            VULKAN_HPP_RAII_NO_EXCEPTIONS
    )
endmacro()