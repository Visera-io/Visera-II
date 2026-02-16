if(NOT VISERA_CORE_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_charted in_target)
    message(STATUS "\nLinking Charted (charted::charted)")

    if(NOT TARGET charted::charted)
        set(CHARTED_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(CHARTED_ENABLE_MODULES ON  CACHE BOOL "" FORCE)
        add_subdirectory("${VISERA_CORE_EXTERNAL_DIR}/Charted")
        set_target_properties(charted PROPERTIES FOLDER "Visera/Core/External/Charted")
    endif()

    target_link_libraries(${in_target} PUBLIC charted::charted)
endmacro()