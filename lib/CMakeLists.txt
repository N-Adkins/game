add_subdirectory(sol2)
add_subdirectory(sdl2)
add_subdirectory(ctre)
add_subdirectory(glm)

find_package(Lua 5.1 REQUIRED)
find_package(OpenGL REQUIRED)

project(
    glad
    LANGUAGES C
)

add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PRIVATE glad/include)

project(
    imgui
    LANGUAGES CXX
)

add_library(imgui STATIC 
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_sdl2.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PRIVATE 
    imgui 
    sdl2/include
)

target_include_directories(game PRIVATE 
    sol2/include 
    sdl2/include
    glad/include
    ctre/include
    imgui
    oof
    glm
    stb
)

target_link_libraries(game PRIVATE 
    sol2 
    ${LUA_LIBRARIES} 
    SDL2::SDL2 
    OpenGL::GL
    glad
    ctre
    imgui
    glm::glm
)
