if(NOT VISERA_STUDIO_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_imgui in_target)
    message(STATUS "\nLinking Dear ImGui (ImGui)")

    if(NOT TARGET ImGui)
        find_package(Vulkan REQUIRED)
        if(NOT TARGET Vulkan::Vulkan)
            message(FATAL_ERROR "Vulkan is required by ImGui!")
        endif()

        if(NOT TARGET glfw)
            message(FATAL_ERROR "GLFW is required by ImGui!")
        endif()

        if(NOT TARGET freetype)
            message(FATAL_ERROR "FreeType is required by ImGui!")
        endif()

        file(GLOB_RECURSE IMGUI_HEADER_FILES "${VISERA_STUDIO_EXTERNAL_DIR}/ImGui/*h")
        file(GLOB_RECURSE IMGUI_SOURCE_FILES "${VISERA_STUDIO_EXTERNAL_DIR}/ImGui/*cpp")
        add_library(ImGui
                    STATIC
                    ${IMGUI_HEADER_FILES}
                    ${IMGUI_SOURCE_FILES})

        message(STATUS "Rasterizer: FreeType")
        target_compile_definitions(ImGui PRIVATE IMGUI_ENABLE_FREETYPE)

        target_link_libraries(ImGui
                              PRIVATE
                              Vulkan::Vulkan
                              glfw
                              freetype
        )
    endif()

    target_link_libraries(${in_target} PUBLIC ImGui)
    target_include_directories(${in_target} PRIVATE "${VISERA_STUDIO_EXTERNAL_DIR}/ImGui")
endmacro()