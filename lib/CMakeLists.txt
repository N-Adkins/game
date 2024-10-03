add_subdirectory(sol2)
add_subdirectory(sdl2)
add_subdirectory(DiligentCore)
add_subdirectory(DiligentTools)
add_subdirectory(DiligentFX)

find_package(Lua 5.1 REQUIRED)
find_package(Vulkan REQUIRED)

target_include_directories(game PRIVATE 
    sol2/include 
    sdl2/include
    DiligentCore
    DiligentTools
    DiligentFX
)

target_compile_definitions(game PRIVATE UNICODE)

target_link_libraries(game PRIVATE 
    sol2 
    ${LUA_LIBRARIES} 
    SDL2::SDL2 
    ${Vulkan_LIBRARIES}
    Diligent-BuildSettings
    Diligent-GraphicsEngineD3D11-shared
    Diligent-GraphicsEngineD3D12-shared
    Diligent-GraphicsEngineOpenGL-shared
    Diligent-GraphicsEngineVk-shared
    DiligentFX
)
