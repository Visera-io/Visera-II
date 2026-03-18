if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please set VISERA_RUNTIME_EXTERNAL_DIR (e.g. from Runtime/CMakeLists.txt)")
endif()

macro(link_imgui in_target)
    message(STATUS "\nLinking Dear ImGui (ImGui)")

    if(NOT TARGET ImGui)
        file(GLOB_RECURSE IMGUI_HEADER_FILES "${VISERA_RUNTIME_EXTERNAL_DIR}/ImGui/*.h")
        file(GLOB_RECURSE IMGUI_SOURCE_FILES "${VISERA_RUNTIME_EXTERNAL_DIR}/ImGui/*.cpp")
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
        set_target_properties(ImGui PROPERTIES FOLDER "Visera/Runtime/External/ImGui")
    endif()

    target_link_libraries(${in_target} PRIVATE "$<BUILD_INTERFACE:ImGui>")
    target_include_directories(${in_target} PRIVATE "${VISERA_RUNTIME_EXTERNAL_DIR}/ImGui")
endmacro()
