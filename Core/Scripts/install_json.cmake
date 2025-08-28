if(NOT VISERA_CORE_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_json in_target)
    message(STATUS "\nLinking JSON (NlohmannJSON)")
    
    if(NOT TARGET NlohmannJSON)
        file(GLOB_RECURSE NOLOHMANN_MODULES ${VISERA_CORE_EXTERNAL_DIR}/NlohmannJSON)
        add_library(NlohmannJSON INTERFACE)
        target_include_directories(NlohmannJSON INTERFACE ${VISERA_CORE_EXTERNAL_DIR}/NlohmannJSON)
    endif()
    
    target_link_libraries(${in_target} PUBLIC NlohmannJSON)
endmacro()