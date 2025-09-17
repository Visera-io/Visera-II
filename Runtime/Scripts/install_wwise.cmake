if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_wwise in_target)
    message(STATUS "\nLinking Wwise (Wwise)")

    if(NOT TARGET Wwise)
        add_subdirectory(${VISERA_RUNTIME_EXTERNAL_DIR}/Wwise)
    endif()

    target_link_libraries(${in_target} PUBLIC Wwise)

    # You must define the AK_OPTIMIZED symbol in the release configuration
    # of your project (it might be called "Ship", "Retail", or something
    # similar in the configuration used to build the retail version of your game).
    # This symbol is used in various places to avoid compiling unnecessary code
    # in the release version.
    target_compile_definitions(${in_target}
        PRIVATE
        AK_OPTIMIZED=$<NOT:$<CONFIG:Debug>>)

    if(APPLE)
        target_link_libraries(${in_target}
            PRIVATE
            "-framework AudioToolbox"
            "-framework AudioUnit"
            "-framework CoreAudio"
            "-framework AVFoundation"
            "-framework CoreMIDI"
            "-framework CoreServices"
        )
    endif()
endmacro()