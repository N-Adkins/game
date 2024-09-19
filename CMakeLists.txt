cmake_minimum_required(VERSION 3.22)

project(
    game
    VERSION 0.1
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(game)
target_include_directories(game PUBLIC src)
target_compile_features(game PUBLIC cxx_std_20)
set_target_properties(game PROPERTIES CXX_EXTENSIONS OFF)

if(NOT MSVC)
    target_compile_options(game PRIVATE -Wall -Wextra -Wconversion -Wno-unused-variable -Wno-unused-private-field -Wno-unused-parameter -Werror)
endif()

if(ASAN)
    target_compile_options(game PRIVATE -g3 -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(game PRIVATE -fsanitize=address,undefined)
endif()

add_subdirectory(lib)
add_subdirectory(shaders)
add_subdirectory(src)
