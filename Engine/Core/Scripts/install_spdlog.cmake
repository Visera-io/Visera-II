if(NOT VISERA_CORE_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_spdlog in_target)
    message(STATUS "\nLinking Spdlog (spdlog::spdlog)")
    
    if(NOT TARGET spdlog::spdlog)
        set(SPDLOG_BUILD_SHARED     OFF CACHE BOOL "" FORCE)
        set(SPDLOG_BUILD_TESTS      OFF CACHE BOOL "" FORCE)
        set(SPDLOG_BUILD_EXAMPLE    OFF CACHE BOOL "" FORCE)
        set(SPDLOG_BUILD_BENCH      OFF CACHE BOOL "" FORCE)
        add_subdirectory(${VISERA_CORE_EXTERNAL_DIR}/Spdlog)
        set_target_properties(spdlog PROPERTIES FOLDER "Visera/Core/External/Spdlog")
    endif()

    target_link_libraries(${in_target} PUBLIC spdlog::spdlog)
endmacro()