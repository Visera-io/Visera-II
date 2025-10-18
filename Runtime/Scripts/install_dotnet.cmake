if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

# Find .NET installation
function(find_dotnet_installation)
    message(STATUS "Searching for .NET installation...")

    # Try to find dotnet executable
    find_program(DOTNET_EXECUTABLE dotnet
        PATHS ENV PATH
        NO_DEFAULT_PATH
    )

    if(NOT DOTNET_EXECUTABLE)
        message(FATAL_ERROR ".NET not found! Please ensure:"
                            "1. .NET is installed from https://dotnet.microsoft.com/download"
                            "2. 'dotnet' command works in your terminal"
                            "3. PATH environment variable is properly set"
                            "4. Try running cmake from the same terminal where 'dotnet --version' works"
                            "Current PATH: $ENV{PATH}")
    endif()
    
    # Get .NET version
    execute_process(
        COMMAND ${DOTNET_EXECUTABLE} --version
        OUTPUT_VARIABLE DOTNET_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    message(STATUS "Found .NET version: ${DOTNET_VERSION}")
    
    # Get .NET installation path
    execute_process(
        COMMAND ${DOTNET_EXECUTABLE} --list-runtimes
        OUTPUT_VARIABLE DOTNET_RUNTIMES_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

     # Extract runtime path from output
     string(REGEX MATCH "Microsoft\\.NETCore\\.App (9\\.[0-9]+\\.[0-9]+) \\[[^]]+\\]" DOTNET_RUNTIME_MATCH "${DOTNET_RUNTIMES_OUTPUT}")

     if(CMAKE_MATCH_1)
         set(DOTNETCORE_RUNTIME_VERSION "${CMAKE_MATCH_1}" PARENT_SCOPE)
         message(STATUS "Found .NETCore runtime version: ${CMAKE_MATCH_1}")
     else()
         message(FATAL_ERROR "Failed to find .NETCore runtime!")
     endif()

    get_filename_component(DOTNET_ROOT_DIR ${DOTNET_EXECUTABLE} DIRECTORY)

    set(DOTNET_ROOT_DIR   ${DOTNET_ROOT_DIR}   PARENT_SCOPE)
    set(DOTNET_EXECUTABLE ${DOTNET_EXECUTABLE} PARENT_SCOPE)
    set(DOTNET_VERSION    ${DOTNET_VERSION}    PARENT_SCOPE)
endfunction()

function(find_nethost_library)
    # Look for nethost library in common locations
    if(WIN32)
        set(NETHOST_LIB_NAMES "nethost.lib")
        set(NETHOST_DLL_NAMES "nethost.dll")
    else()
        set(NETHOST_LIB_NAMES "libnethost.a" "libnethost.so" "libnethost.dylib")
    endif()

    # Search paths for nethost library
    set(NETHOST_SEARCH_PATHS
        "${DOTNET_ROOT_DIR}/host/fxr"
        "${DOTNET_ROOT_DIR}/packs/Microsoft.NETCore.App.Host.win-x64/${DOTNETCORE_RUNTIME_VERSION}/runtimes/win-x64/native"
        "${DOTNET_ROOT_DIR}/packs/Microsoft.NETCore.App.Host.linux-x64/${DOTNETCORE_RUNTIME_VERSION}/runtimes/linux-x64/native"
        "${DOTNET_ROOT_DIR}/packs/Microsoft.NETCore.App.Host.osx-arm64/${DOTNETCORE_RUNTIME_VERSION}/runtimes/osx-arm64/native"
    )

    find_library(NETHOST_LIBRARY
        NAMES ${NETHOST_LIB_NAMES}
        PATHS ${NETHOST_SEARCH_PATHS}
        NO_DEFAULT_PATH
    )

    if(WIN32 AND NETHOST_LIBRARY)
        # Also find the DLL for runtime
        find_file(NETHOST_DLL
            NAMES ${NETHOST_DLL_NAMES}
            PATHS ${NETHOST_SEARCH_PATHS}
            NO_DEFAULT_PATH
        )
        if(NOT NETHOST_DLL)
            message(FATAL_ERROR "Failed to find the nethost DLL!")
        endif()
    endif()
    
    if(NOT NETHOST_LIBRARY)
        message(FATAL_ERROR "Failed to find the nethost library!")
    endif()

    message(STATUS "Found nethost library: ${NETHOST_LIBRARY}")

    set(NETHOST_LIBRARY "${NETHOST_LIBRARY}" PARENT_SCOPE)
    if(WIN32)
        message(STATUS "Found nethost DLL: ${NETHOST_DLL}")
        set(NETHOST_DLL "${NETHOST_DLL}" PARENT_SCOPE)
    endif()
endfunction()

function(find_hostfxr_library)
    # Look for nethost library in common locations
    if(WIN32)
        set(HOSTFXR_DLL_NAMES "hostfxr.dll")
    else()
        set(HOSTFXR_DLL_NAMES "libhostfxr.a" "libhostfxr.so" "libhostfxr.dylib")
    endif()

    # Search paths for hostfxr library
    set(HOSTFXR_SEARCH_PATHS
            "${DOTNET_ROOT_DIR}/host/fxr/${DOTNETCORE_RUNTIME_VERSION}"
    )

    find_file(HOSTFXR_DLL
            NAMES ${HOSTFXR_DLL_NAMES}
            PATHS ${HOSTFXR_SEARCH_PATHS}
            NO_DEFAULT_PATH)

    if(NOT HOSTFXR_DLL)
        message(FATAL_ERROR "Failed to find the hostfxr DLL!")
    endif()

    message(STATUS "Found hostfxr DLL: ${HOSTFXR_DLL}")

    set(HOSTFXR_DLL "${HOSTFXR_DLL}" PARENT_SCOPE)
endfunction()

macro(link_dotnet in_target)
    message(STATUS "\nLinking .NET (.NET)")

    find_dotnet_installation()
    find_nethost_library()
    find_hostfxr_library()

    if(WIN32)
        add_compile_definitions(HOSTFXR_LIBRARY_NAME=u8"hostfxr.dll")
    elseif(APPLE)
        add_compile_definitions(HOSTFXR_LIBRARY_NAME=u8"libhostfxr.dylib")
    else()
        message(FATAL_ERROR "Unsupported platform!")
    endif ()

    target_include_directories(${in_target} PRIVATE ${VISERA_RUNTIME_EXTERNAL_DIR}/DotNET)
    target_link_libraries(${in_target} PRIVATE ${NETHOST_LIBRARY})

    add_custom_command(
        TARGET ${in_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${HOSTFXR_DLL}
        $<TARGET_FILE_DIR:${VISERA_APP}>
    )
endmacro()