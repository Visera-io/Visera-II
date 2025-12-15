if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_dotnet in_target)
    message(STATUS "\nLinking .NET Runtime (DotNETRuntime)")

    target_include_directories(${in_target} PRIVATE "${VISERA_RUNTIME_EXTERNAL_DIR}/DotNET")

    if(NOT TARGET DotNETRuntime)
        add_subdirectory("${VISERA_RUNTIME_EXTERNAL_DIR}/DotNET")
        add_compile_definitions(
            VISERA_DOTNET_ROOT=\"${VISERA_DOTNET_RELATIVE_PATH}\"
            VISERA_DOTNET_HOSTFXR_PATH=\"${VISERA_DOTNET_RELATIVE_PATH}/host/fxr/${VISERA_DOTNET_VERSION}/${VISERA_DOTNET_HOSTFXR_NAME}\"
        )
    endif()

    add_dependencies(${in_target} DotNETRuntime)
    target_include_directories(${in_target} PRIVATE "${VISERA_RUNTIME_EXTERNAL_DIR}/DotNET/Include")
endmacro()