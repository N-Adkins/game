project(engine
    LANGUAGES C
)

add_library(engine SHARED)

target_include_directories(engine PRIVATE src)
target_compile_definitions(engine PRIVATE LENGINE_BUILD)

if (NOT WIN32)
    find_package(X11 REQUIRED)
    target_link_libraries(engine PRIVATE ${X11_LIBRARIES})

    target_compile_options(engine PRIVATE
        -Wall -Wformat -Wextra -Werror -fdiagnostics-color=always 
    )

    if (CMAKE_C_COMPILER_ID STREQUAL "Clang") 
        message("Using clang, enabling more warnings on engine")
        target_compile_options(engine PRIVATE ${LESS_CLANG_WARNINGS})
    endif()

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        message("Building in debug mode, enabling sanitizer on engine")
        target_compile_options(engine PRIVATE -fsanitize=address,undefined)
        target_link_options(engine PRIVATE -fsanitize=address,undefined)
    endif()

endif()

add_subdirectory(src)

get_target_property(SOURCES engine SOURCES)

foreach(SOURCE_FILE IN LISTS SOURCES)
    # Get the relative path from the project root
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src" "${SOURCE_FILE}")
    
    # Get just the file name
    get_filename_component(FILE_NAME "${SOURCE_FILE}" NAME)
    
    # Set the compile definitions for each source file
    set_source_files_properties(
        ${SOURCE_FILE}
        PROPERTIES
        COMPILE_DEFINITIONS "LFILE_PATH=\"${REL_PATH}\";LFILE_NAME=\"${FILE_NAME}\""
    )
endforeach()
