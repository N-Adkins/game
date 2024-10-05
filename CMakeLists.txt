cmake_minimum_required(VERSION 3.22)

project(
    game
    VERSION 0.1
    LANGUAGES CXX C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(game)
target_include_directories(game PUBLIC src)
target_compile_features(game PUBLIC cxx_std_20)
set_target_properties(game PROPERTIES CXX_EXTENSIONS OFF)

target_compile_options(game PRIVATE -Wall -Wextra -Wconversion -Wno-unused-variable -Wno-unused-private-field -Wno-unused-parameter -Werror)

add_subdirectory(lib)
add_subdirectory(src)

# Symlink resources to build directory
add_custom_command(
    TARGET game 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/resources
    COMMENT "Creating symlink to resources directory in build directory"
)
