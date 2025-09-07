if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_dotnet in_target)
    message(STATUS "\nLinking .NET (.NET)")

    target_include_directories(${in_target} PUBLIC ${VISERA_RUNTIME_EXTERNAL_DIR}/DotNET)
endmacro()