target_sources(engine PRIVATE
    defines.c
)

add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(containers)
