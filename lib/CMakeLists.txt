add_subdirectory(sol2)
add_subdirectory(sdl2)
add_subdirectory(ctre)

find_package(Lua 5.1 REQUIRED)
find_package(OpenGL REQUIRED)

project(
    glad
    LANGUAGES C
)

add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PRIVATE glad/include)

target_include_directories(game PRIVATE 
    sol2/include 
    sdl2/include
    glad/include
    ctre/include
)

target_link_libraries(game PRIVATE 
    sol2 
    ${LUA_LIBRARIES} 
    SDL2::SDL2 
    OpenGL::GL
    glad
    ctre
)
