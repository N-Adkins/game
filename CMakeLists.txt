cmake_minimum_required(VERSION 3.20)

project(
    less
    LANGUAGES C
)

if (NOT WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message("Building in debug mode, enabling clang-tidy")
        set(CMAKE_C_CLANG_TIDY clang-tidy --quiet --use-color -warnings-as-errors=*)
    endif()
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

set(LESS_CLANG_OPTIONS 
    -Weverything
    -Wno-switch-enum
    -Wno-switch-default
    -Wno-signed-enum-bitfield
    -Wno-deprecated-register
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-c++98-c++11-compat-pedantic
    -Wno-nested-anon-types
    -Wno-gnu-anonymous-struct
    -Wno-missing-prototypes
    -Wno-documentation
    -Wno-unused-const-variable
    -Wno-format-nonliteral
    -Wno-error=padded 
    -Wno-declaration-after-statement
    -Wno-unsafe-buffer-usage
    -Wno-pedantic
)

add_subdirectory(editor)
add_subdirectory(engine)
