cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

add_library(VulkanModule)
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
if(APPLE)
add_library(VulkanLoader SHARED IMPORTED GLOBAL)
set_target_properties(VulkanLoader PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/MacOS/libvulkan.1.dylib"
)
endif()