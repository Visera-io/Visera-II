if(NOT VISERA_FORGE_EXTERNAL_DIR)
    message(FATAL_ERROR "Please include 'install.cmake' before installing any package!")
endif()

macro(link_slang in_target)
    message(STATUS "Linking Slang (Slang)")

    if(NOT TARGET Slang)
        add_subdirectory(${VISERA_FORGE_EXTERNAL_DIR}/Slang)
        target_sources(Slang PRIVATE ${SLANG_DLL_PATH})
        set_target_properties(Slang PROPERTIES FOLDER "Visera/Forge/External/Slang")
    endif()

    target_link_libraries(${in_target} PRIVATE Slang)
    
    add_custom_command(
        TARGET ${in_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SLANG_DLL_PATH}
        $<TARGET_FILE_DIR:${in_target}>
    )
    if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(
        TARGET ${in_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SLANG_PDB_PATH}
        $<TARGET_FILE_DIR:${in_target}>
    )
    endif()
endmacro()