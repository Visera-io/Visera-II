cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

add_library(VulkanModule STATIC
                         ${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanLoader.cpp)
target_sources(VulkanModule PUBLIC
    FILE_SET  CXX_MODULES
    FILES     ${CMAKE_CURRENT_SOURCE_DIR}/Include/vulkan/vulkan.cppm
)
target_include_directories(VulkanModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)
target_compile_definitions(VulkanModule PRIVATE
    VULKAN_HPP_NO_STD_MODULE
    VULKAN_HPP_NO_SMART_HANDLE
    VULKAN_HPP_NO_EXCEPTIONS
    VULKAN_HPP_RAII_NO_EXCEPTIONS
)
target_compile_definitions(VulkanModule PUBLIC
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)
message(STATUS "Vulkan files will be copied into the app bundle.")
if(APPLE)
    set(VULKAN_ARCHIVE     "${CMAKE_CURRENT_SOURCE_DIR}/MacOS.zip")
    set(VULKAN_EXTRACT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/MacOS")
    if(NOT EXISTS "${VULKAN_EXTRACT_DIR}/libMoltenVK.dylib")
        message(STATUS "Extracting ${VULKAN_ARCHIVE}")
        file(ARCHIVE_EXTRACT
            INPUT       "${VULKAN_ARCHIVE}"
            DESTINATION "${VULKAN_EXTRACT_DIR}"
        )
    endif()

    add_library(VulkanLoader SHARED IMPORTED GLOBAL)
    set_target_properties(VulkanLoader PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/MacOS/libvulkan.1.dylib"
    )
elseif(WIN32)
    set(VULKAN_ARCHIVE     "${CMAKE_CURRENT_SOURCE_DIR}/Windows.zip")
    set(VULKAN_EXTRACT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Windows")
    if(NOT EXISTS "${VULKAN_EXTRACT_DIR}/vulkan-1.lib")
    message(STATUS "Extracting ${VULKAN_ARCHIVE}")
    file(ARCHIVE_EXTRACT
         INPUT       "${VULKAN_ARCHIVE}"
         DESTINATION "${VULKAN_EXTRACT_DIR}"
    )
    endif()

    add_library(VulkanLoader UNKNOWN IMPORTED GLOBAL)
    set(VULKAN_LIB "${CMAKE_CURRENT_SOURCE_DIR}/Windows/vulkan-1.lib")
    set_target_properties(VulkanLoader PROPERTIES
        IMPORTED_CONFIGURATIONS   "Debug;Release;Develop"
        IMPORTED_LOCATION_DEBUG   "${VULKAN_LIB}"
        IMPORTED_LOCATION_RELEASE "${VULKAN_LIB}"
        IMPORTED_LOCATION_DEVELOP "${VULKAN_LIB}"
    )
else()
    mssage(FATAL_ERROR "Unsupported platform!")
endif()

target_link_libraries(VulkanModule PUBLIC VulkanLoader)

if(APPLE)
    add_custom_command(TARGET VulkanModule POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Frameworks"
        "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Resources/Vulkan/"
        # Vulkan
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/MacOS/libvulkan.1.dylib" # Note: this is a renamed lib
        "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Frameworks/"
        # MoltenVk
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/MacOS/libMoltenVK.dylib"
        "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Frameworks/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/MacOS/MoltenVK_icd.json"
        "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Resources/Vulkan/"
    )
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        add_custom_command(TARGET VulkanModule POST_BUILD
            # Validation Layer
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/MacOS/libVkLayer_khronos_validation.dylib"
            "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Frameworks/"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/MacOS/VkLayer_khronos_validation.json"
            "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Resources/Vulkan/"
        )
    endif()
elseif(WIN32)
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        add_custom_command(TARGET VulkanModule POST_BUILD
            # Validation Layer
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/Windows/VkLayer_khronos_validation.dll"
            "${VISERA_APP_FRAMEWORK_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/Windows/VkLayer_khronos_validation.json"
            "${VISERA_APP_FRAMEWORK_DIR}"
        )
        add_custom_command(TARGET VulkanModule POST_BUILD
            # PDB files
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/Windows/VkLayer_khronos_validation.pdb"
            "${VISERA_APP_FRAMEWORK_DIR}"
        )
    endif()
else()
    mssage(FATAL_ERROR "Unsupported platform!")
endif()