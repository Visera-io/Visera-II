if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_box2d in_target)
    message(STATUS "\nLinking Box2D (box2d)")

    if(NOT TARGET box2d)
        add_subdirectory(${VISERA_RUNTIME_EXTERNAL_DIR}/Box2D)
        set_target_properties(box2d PROPERTIES FOLDER "Visera/Runtime/External/Box2D")
    endif()

    target_link_libraries(${in_target} PRIVATE box2d)
endmacro()