add_subdirectory(sol2)
add_subdirectory(bgfx)
add_subdirectory(sdl2)

find_package(Lua 5.1 REQUIRED)
find_package(Vulkan REQUIRED)

target_include_directories(game PRIVATE sol2/include bgfx/bgfx/include bgfx/bx/include bgfx/bimg/include sdl2/include)
target_link_libraries(game PRIVATE sol2 ${LUA_LIBRARIES} bgfx bimg bx SDL2::SDL2 ${Vulkan_LIBRARIES})
