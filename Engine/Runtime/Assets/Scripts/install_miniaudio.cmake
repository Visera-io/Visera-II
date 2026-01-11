if(NOT VISERA_ASSETS_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_miniaudio in_target)
    message(STATUS "\nLinking MiniAudio (MiniAudio)")
    
    if(NOT TARGET MiniAudio)
        add_library(MiniAudio INTERFACE)
        target_include_directories(MiniAudio INTERFACE
            "${VISERA_ASSETS_EXTERNAL_DIR}/MiniAudio")
        set_target_properties(MiniAudio PROPERTIES FOLDER "Visera/Assets/External/MiniAudio")
    endif()

    target_link_libraries(${in_target} PRIVATE MiniAudio)
endmacro()