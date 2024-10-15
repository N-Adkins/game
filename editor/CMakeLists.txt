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
endif()

#[[
add_custom_command(TARGET editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:engine>"
    "$<TARGET_FILE_DIR:editor>")
]]

add_subdirectory(src)
