if(NOT VISERA_GRAPHICS_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_bvh in_target)
    message(STATUS "\nLinking BVH (bvh)")

    if(NOT TARGET bvh)
        add_subdirectory(${VISERA_GRAPHICS_EXTERNAL_DIR}/BVH)
        target_sources(bvh PRIVATE "${VISERA_GRAPHICS_EXTERNAL_DIR}/BVH/CMakeLists.txt")
        set_target_properties(bvh PROPERTIES FOLDER "Visera/Graphics/External/BVH")
    endif()

    target_link_libraries(${in_target} PRIVATE bvh)
endmacro()