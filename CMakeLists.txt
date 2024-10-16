cmake_minimum_required(VERSION 3.20)

project(
    less
    LANGUAGES C
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Building in debug mode, enabling clang-tidy")
    set(CMAKE_C_CLANG_TIDY clang-tidy --quiet --use-color -warnings-as-errors=*)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

add_subdirectory(editor)
add_subdirectory(engine)
