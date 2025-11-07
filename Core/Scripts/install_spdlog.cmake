if(NOT VISERA_CORE_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_spdlog in_target)
    message(STATUS "\nLinking Spdlog (spdlog::spdlog)")
    
    if(NOT TARGET spdlog)
        add_subdirectory(${VISERA_CORE_EXTERNAL_DIR}/Spdlog)
        set_target_properties(spdlog PROPERTIES FOLDER "Visera/Core/External/Spdlog")
    endif()
    
    target_link_libraries(${in_target} PUBLIC spdlog::spdlog)
endmacro()