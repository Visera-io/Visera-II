if(NOT VISERA_ENGINE_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_wwise in_target)
    message(STATUS "\nLinking Wwise (Wwise)")

    if(NOT TARGET Wwise)
        add_subdirectory(${VISERA_ENGINE_EXTERNAL_DIR}/Wwise)
        set_target_properties(Wwise PROPERTIES FOLDER "Visera/Engine/External/Wwise")
        set_target_properties(WwiseSamples PROPERTIES FOLDER "Visera/Engine/External/Wwise")
    endif()

    target_link_libraries(${in_target} PUBLIC
            Wwise
            WwiseSamples
    )
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        if(WIN32)
            target_link_libraries(${in_target}
                    PRIVATE
                    Ws2_32 # Wwise Communication uses Windows Sockets (Winsock) API
            )
        endif()
    endif()

    if(APPLE)
        target_link_libraries(${in_target}
            PRIVATE
            "-framework AudioToolbox"
            "-framework AudioUnit"
            "-framework CoreAudio"
            "-framework AVFoundation"
            "-framework CoreMIDI"
            "-framework CoreServices"
            "-framework Foundation"
            "-framework CoreFoundation"
        )
    endif()
endmacro()