project(editor
    LANGUAGES C
)

add_executable(editor)

target_link_libraries(editor PRIVATE engine)
target_include_directories(editor PRIVATE ../engine/src)

if (NOT WIN32)
    target_compile_options(editor PRIVATE
        -Wall -Wextra -Werror -fdiagnostics-color=always
    )

    if (CMAKE_C_COMPILER_ID STREQUAL "Clang") 
        message("Using clang, enabling more warnings on editor")
        target_compile_options(editor PRIVATE ${LESS_CLANG_WARNINGS})
    endif()
    
    if (LESS_SANITIZE)
        message("Enabling sanitizer on editor")
        target_compile_options(editor PRIVATE -fsanitize=address,undefined)
        target_link_options(editor PRIVATE -fsanitize=address,undefined)
    endif()
endif()

add_custom_command(TARGET editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:engine>"
    "$<TARGET_FILE_DIR:editor>")

add_subdirectory(src)

get_target_property(SOURCES editor SOURCES)

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
