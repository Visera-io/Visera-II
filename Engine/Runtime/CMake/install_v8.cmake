if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please set VISERA_RUNTIME_EXTERNAL_DIR (e.g. from Runtime/CMakeLists.txt)")
endif()

macro(link_v8 in_target)
    message(STATUS "\nLinking V8 (ViseraEXT::V8)")

    if(NOT TARGET ViseraEXT::V8)
        set(V8_BASE_DIR "${VISERA_RUNTIME_EXTERNAL_DIR}/V8")
        set(V8_EXTRACT_DIR "${V8_BASE_DIR}")
        if(APPLE)
            set(V8_ARCHIVE "${V8_BASE_DIR}/MacOS.zip")
            if(NOT EXISTS "${V8_EXTRACT_DIR}/MacOS/libv8_monolith.a")
                if(EXISTS "${V8_ARCHIVE}")
                    message(STATUS "Extracting ${V8_ARCHIVE}")
                    file(ARCHIVE_EXTRACT
                        INPUT       "${V8_ARCHIVE}"
                        DESTINATION "${V8_EXTRACT_DIR}"
                    )
                else()
                    message(FATAL_ERROR "V8 archive not found: ${V8_ARCHIVE}")
                endif()
            endif()
        elseif(WIN32)
            set(V8_ARCHIVE "${V8_BASE_DIR}/Windows.zip")
            if(NOT EXISTS "${V8_EXTRACT_DIR}/Windows/Release/v8_monolith.lib" AND NOT EXISTS "${V8_EXTRACT_DIR}/Windows/v8_monolith.lib")
                if(EXISTS "${V8_ARCHIVE}")
                    message(STATUS "Extracting ${V8_ARCHIVE}")
                    file(ARCHIVE_EXTRACT
                        INPUT       "${V8_ARCHIVE}"
                        DESTINATION "${V8_EXTRACT_DIR}"
                    )
                else()
                    message(FATAL_ERROR "V8 archive not found: ${V8_ARCHIVE}")
                endif()
            endif()
        else()
            message(FATAL_ERROR "V8: Unsupported platform (only APPLE and WIN32 are supported)")
        endif()

        add_subdirectory("${V8_BASE_DIR}")
        set_target_properties(ViseraEXT::V8 PROPERTIES FOLDER "Visera/Runtime/External/V8")
    endif()

    target_link_libraries(${in_target} PRIVATE "$<BUILD_INTERFACE:ViseraEXT::V8>")
endmacro()
