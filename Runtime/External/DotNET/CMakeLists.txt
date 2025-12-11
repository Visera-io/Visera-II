cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

if(APPLE)
    set(DOTNET_ARCHIVE     "${CMAKE_CURRENT_SOURCE_DIR}/MacOS.zip")
    set(DOTNET_EXTRACT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/MacOS")
    set(DOTNET_COPY_DST "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Frameworks/DotNETRuntime")
    set(DOTNET_RELATIVE_PATH "../Frameworks/DotNETRuntime")
    set(DOTNET_HOSTFXR_NAME  "libhostfxr.dylib")
elseif(WIN32)
    set(DOTNET_ARCHIVE     "${CMAKE_CURRENT_SOURCE_DIR}/Windows.zip")
    set(DOTNET_EXTRACT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Windows")
    set(DOTNET_COPY_DST "${VISERA_APP_FRAMEWORK_DIR}/DotNETRuntime")
    set(DOTNET_RELATIVE_PATH "DotNETRuntime")
    set(DOTNET_HOSTFXR_NAME  "hostfxr.dll")
else()
    mssage(FATAL_ERROR "Unsupported platform!")
endif()

if(NOT EXISTS "${DOTNET_EXTRACT_DIR}")
    message(STATUS "Extracting ${DOTNET_ARCHIVE}")
    file(ARCHIVE_EXTRACT
        INPUT       "${DOTNET_ARCHIVE}"
        DESTINATION "${DOTNET_EXTRACT_DIR}"
    )
endif()

add_custom_target(DotNETRuntime ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DOTNET_COPY_DST}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${DOTNET_EXTRACT_DIR}" "${DOTNET_COPY_DST}"
)

# Detect .NET Runtime version (assume exactly one)
file(GLOB _appver_dirs LIST_DIRECTORIES true  "${DOTNET_EXTRACT_DIR}/shared/Microsoft.NETCore.App/*")
list(LENGTH _appver_dirs _n)
if(_n EQUAL 0)
    message(FATAL_ERROR "Failed to detect .NET version: no directory under ${DOTNET_EXTRACT_DIR}/shared/Microsoft.NETCore.App/")
endif()
list(GET _appver_dirs 0 _ver_dir)
get_filename_component(DOTNET_VERSION "${_ver_dir}" NAME)

set(VISERA_DOTNET_VERSION "${DOTNET_VERSION}"                   PARENT_SCOPE)
message(STATUS ".NET Runtime version: ${DOTNET_VERSION}")
set(VISERA_DOTNET_RELATIVE_PATH "${DOTNET_RELATIVE_PATH}"       PARENT_SCOPE)
message(STATUS ".NET Runtime path: <Application>${DOTNET_RELATIVE_PATH}")
set(VISERA_DOTNET_HOSTFXR_NAME "${DOTNET_HOSTFXR_NAME}"         PARENT_SCOPE)
message(STATUS ".NET HostFXR name: ${DOTNET_HOSTFXR_NAME}")

message(STATUS ".NET Runtime will be copied into the app bundle.")