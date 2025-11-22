if(NOT VISERA_ENGINE_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_entt in_target)
    message(STATUS "\nLinking EnTT (EnTT::EnTT)")

    if(NOT TARGET EnTT)
        add_subdirectory(${VISERA_ENGINE_EXTERNAL_DIR}/EnTT)
        set_target_properties(EnTT PROPERTIES FOLDER "Visera/Engine/External/EnTT")
    endif()

    target_link_libraries(${in_target} PUBLIC EnTT::EnTT)
endmacro()