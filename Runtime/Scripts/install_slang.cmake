if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package.")
endif()

macro(link_slang in_target)
    message(STATUS "\nLinking Slang (slang)")
    # Make sure that Version(VulkanSDK) >= "1.4.0"
    add_library(slang UNKNOWN IMPORTED)

    if (WIN32)
        set_target_properties(slang
                PROPERTIES
                IMPORTED_LOCATION "$ENV{VULKAN_SDK}/Lib/slang.lib"
                INTERFACE_INCLUDE_DIRECTORIES "$ENV{VULKAN_SDK}/Include/slang")
        target_link_libraries(${in_target} PUBLIC slang)

    elseif (APPLE) # MacOS
        set_target_properties(slang
                PROPERTIES
                IMPORTED_LOCATION "$ENV{VULKAN_SDK}/lib/libslang.dylib"
                INTERFACE_INCLUDE_DIRECTORIES "$ENV{VULKAN_SDK}/include/slang")
        target_link_libraries(${in_target} PUBLIC slang)

        #elseif (LINUX)
    else()
        message(FATAL_ERROR "[Slang] Unsupported Platform!")
    endif()

endmacro()