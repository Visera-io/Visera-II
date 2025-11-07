if(NOT VISERA_ENGINE_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_slang in_target)
    message(STATUS "\nLinking Slang (Slang)")

    if(NOT TARGET Slang)
        add_subdirectory(${VISERA_ENGINE_EXTERNAL_DIR}/Slang)
        set_target_properties(Slang PROPERTIES FOLDER "Visera/Engine/External/Slang")
    endif()

    target_link_libraries(${in_target} PUBLIC Slang)
    add_custom_command(
            TARGET ${in_target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SLANG_DLL_PATH}
            $<TARGET_FILE_DIR:${VISERA_APP}>
    )
    if(WIN32 AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(
        TARGET ${in_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SLANG_PDB_PATH}
        $<TARGET_FILE_DIR:${VISERA_APP}>
    )
    endif()
endmacro()