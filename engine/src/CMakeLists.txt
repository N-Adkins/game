target_sources(engine PRIVATE
    entry.c
    defines.c
)

add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(containers)
