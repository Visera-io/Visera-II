if(NOT VISERA_CORE_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_json in_target)
    message(STATUS "\nLinking NlohmannJSON (nlohmann_json::nlohmann_json)")
    
    if(NOT TARGET nlohmann_json::nlohmann_json)
        set(NLOHMANN_JSON_BUILD_MODULES ON  CACHE BOOL "" FORCE)
        set(JSON_GlobalUDLs             OFF CACHE BOOL "" FORCE)
        set(JSON_BuildTests             OFF CACHE BOOL "" FORCE)
        add_subdirectory("${VISERA_CORE_EXTERNAL_DIR}/NlohmannJSON")
        set_target_properties(nlohmann_json_modules PROPERTIES FOLDER "Visera/Core/External/NlohmannJSON")
    endif()
    
    target_link_libraries(${in_target} PUBLIC nlohmann_json_modules)
endmacro()