if(NOT VISERA_UI_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before install_imgui")
endif()

macro(link_imgui in_target)
    message(STATUS "\nLinking Dear ImGui (ImGui)")

    if(NOT TARGET ImGui)
        file(GLOB_RECURSE IMGUI_HEADER_FILES "${VISERA_UI_EXTERNAL_DIR}/ImGui/*h")
        file(GLOB_RECURSE IMGUI_SOURCE_FILES "${VISERA_UI_EXTERNAL_DIR}/ImGui/*cpp")
        add_library(ImGui STATIC
                    ${IMGUI_HEADER_FILES}
                    ${IMGUI_SOURCE_FILES})

        message(STATUS "Rasterizer: FreeType")
        target_compile_definitions(ImGui PRIVATE IMGUI_ENABLE_FREETYPE)

        target_link_libraries(ImGui
                              PRIVATE
                              VulkanModule
                              glfw
                              freetype
        )
        if(APPLE)
            target_compile_options(ImGui PRIVATE $<$<CONFIG:Debug>:-g0> $<$<CONFIG:Develop>:-g0>)
        endif()
        set_target_properties(ImGui PROPERTIES FOLDER "Visera/UI/External/ImGui")
    endif()

    target_link_libraries(${in_target} PRIVATE "$<BUILD_INTERFACE:ImGui>")
    target_include_directories(${in_target} PRIVATE "${VISERA_UI_EXTERNAL_DIR}/ImGui")
endmacro()
