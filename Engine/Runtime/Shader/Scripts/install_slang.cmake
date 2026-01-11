if(NOT VISERA_SHADER_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_slang in_target)
    message(STATUS "\nLinking Slang (Slang)")

    if(NOT TARGET Slang)
        add_subdirectory(${VISERA_SHADER_EXTERNAL_DIR}/Slang)
        target_sources(Slang PRIVATE ${SLANG_DLL_PATH})
        set_target_properties(Slang PROPERTIES FOLDER "${VISERA_SHADER}/External/Slang")
    endif()

    target_link_libraries(${in_target} PUBLIC Slang)
    
    add_custom_command(
        TARGET ${in_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SLANG_DLL_PATH}
        ${VISERA_APP_FRAMEWORK_DIR}
    )
    if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(
        TARGET ${in_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SLANG_PDB_PATH}
        ${VISERA_APP_FRAMEWORK_DIR}
    )
    endif()
endmacro()