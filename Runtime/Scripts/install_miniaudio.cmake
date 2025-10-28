if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_miniaudio in_target)
    message(STATUS "\nLinking MiniAudio (MiniAudio)")
    
    if(NOT TARGET MiniAudio)
        add_library(MiniAudio INTERFACE)
        target_include_directories(MiniAudio INTERFACE
            "${VISERA_RUNTIME_EXTERNAL_DIR}/MiniAudio")
    endif()

    target_link_libraries(${in_target} PUBLIC MiniAudio)
endmacro()