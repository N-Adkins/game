add_subdirectory(sol2)
#add_subdirectory(luajit)

find_package(Lua 5.1 REQUIRED)

target_include_directories(game PRIVATE sol2/include)
target_link_libraries(game PRIVATE sol2 ${LUA_LIBRARIES})
