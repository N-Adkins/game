add_subdirectory(sol2)
add_subdirectory(luajit)

target_include_directories(game PRIVATE sol2/include)
target_link_libraries(game PRIVATE sol2 libluajit)
