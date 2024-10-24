target_sources(engine PRIVATE
    defines.c
)

add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(containers)
add_subdirectory(renderer)
add_subdirectory(utils)
