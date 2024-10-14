cmake_minimum_required(VERSION 3.20)

project(
    less
    LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

add_subdirectory(editor)
add_subdirectory(engine)
