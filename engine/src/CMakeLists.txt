target_sources(engine PRIVATE
    entry.c
)

add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(containers)
