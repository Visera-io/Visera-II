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
        message(FATAL_ERROR ".NET not found! Please ensure:
1. .NET is installed from https://dotnet.microsoft.com/download
2. 'dotnet' command works in your terminal
3. PATH environment variable is properly set
4. Try running cmake from the same terminal where 'dotnet --version' works

Current PATH: $ENV{PATH}")
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
    string(REGEX MATCH "Microsoft\\.NETCore\\.App 9\\.[0-9]+\\.[0-9]+ \\[([^]]+)\\]" DOTNET_RUNTIME_MATCH "${DOTNET_RUNTIMES_OUTPUT}")

    if(CMAKE_MATCH_1)
        set(DOTNET_ROOT_PATH "${CMAKE_MATCH_1}" PARENT_SCOPE)
        message(STATUS "Found .NET root path: ${CMAKE_MATCH_1}")
    else()
        message(FATAL_ERROR "Failed to find the .NET runtime from ${DOTNET_RUNTIMES_OUTPUT}!")
    endif()

    set(DOTNET_EXECUTABLE "${DOTNET_EXECUTABLE}" PARENT_SCOPE)
    set(DOTNET_VERSION "${DOTNET_VERSION}" PARENT_SCOPE)
endfunction()

# Find nethost library
function(find_nethost_library)
    find_dotnet_installation()
    
    # Look for nethost library in common locations
    if(WIN32)
        set(NETHOST_LIB_NAMES "nethost.lib")
        set(NETHOST_DLL_NAMES "nethost.dll")
    else()
        set(NETHOST_LIB_NAMES "libnethost.a" "libnethost.so" "libnethost.dylib")
    endif()
    
    # Search paths for nethost library
    set(NETHOST_SEARCH_PATHS
        "${DOTNET_ROOT_PATH}/host/fxr"
        "${DOTNET_ROOT_PATH}/../packs/Microsoft.NETCore.App.Host.win-x64/${DOTNET_VERSION}/runtimes/win-x64/native"
        "${DOTNET_ROOT_PATH}/../packs/Microsoft.NETCore.App.Host.linux-x64/${DOTNET_VERSION}/runtimes/linux-x64/native"
        "${DOTNET_ROOT_PATH}/../packs/Microsoft.NETCore.App.Host.osx-x64/${DOTNET_VERSION}/runtimes/osx-x64/native"
    )

    # Find the library
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
    endif()
    
    if(NOT NETHOST_LIBRARY)
        message(WARNING "nethost library not found in standard locations. You may need to:")
        message(WARNING "1. Install .NET SDK (not just runtime)")
        message(WARNING "2. Manually download nethost from: https://www.nuget.org/packages/Microsoft.NETCore.DotNetAppHost")
        message(WARNING "3. Set NETHOST_LIBRARY_PATH manually")
        
        # Try manual path if provided
        if(NETHOST_LIBRARY_PATH)
            find_library(NETHOST_LIBRARY
                NAMES ${NETHOST_LIB_NAMES}
                PATHS ${NETHOST_LIBRARY_PATH}
                NO_DEFAULT_PATH
            )
        endif()
    endif()
    
    set(NETHOST_LIBRARY "${NETHOST_LIBRARY}" PARENT_SCOPE)
    if(WIN32)
        set(NETHOST_DLL "${NETHOST_DLL}" PARENT_SCOPE)
    endif()
endfunction()

macro(link_dotnet in_target)
    message(STATUS "\nLinking .NET (.NET)")
    
    # Find .NET and nethost library
    find_nethost_library()
    
    # Include headers from our External/DotNET directory
    target_include_directories(${in_target} PUBLIC ${VISERA_RUNTIME_EXTERNAL_DIR}/DotNET)
    
    if(NETHOST_LIBRARY)
        message(STATUS "Found nethost library: ${NETHOST_LIBRARY}")
        target_link_libraries(${in_target} PRIVATE ${NETHOST_LIBRARY})
        
        # Copy DLL to output directory on Windows
        if(WIN32 AND NETHOST_DLL)
            message(STATUS "Found nethost DLL: ${NETHOST_DLL}")
            add_custom_command(TARGET ${in_target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${NETHOST_DLL}
                    $<TARGET_FILE_DIR:${in_target}>
            )
        endif()
        
        # Define preprocessor macros for the target
        target_compile_definitions(${in_target} PRIVATE VISERA_DOTNET_ENABLED=1)
        
    else()
        message(WARNING "nethost library not found! .NET scripting will be disabled.")
        message(WARNING "To enable .NET scripting:")
        message(WARNING "1. Install .NET 8.0 SDK from https://dotnet.microsoft.com/download")
        message(WARNING "2. Or manually set NETHOST_LIBRARY_PATH to the location of nethost library")
        
        target_compile_definitions(${in_target} PRIVATE VISERA_DOTNET_ENABLED=0)
    endif()
    
    # Store variables for potential use by other parts of the build
    set(VISERA_DOTNET_ROOT_PATH "${DOTNET_ROOT_PATH}" PARENT_SCOPE)
    set(VISERA_DOTNET_VERSION "${DOTNET_VERSION}" PARENT_SCOPE)
    set(VISERA_DOTNET_EXECUTABLE "${DOTNET_EXECUTABLE}" PARENT_SCOPE)
endmacro()