target_sources(game PRIVATE
    main.cpp
    platform.cpp
    logging.cpp
    pch.cpp
)

target_precompile_headers(game PRIVATE pch.hpp)

add_subdirectory(gfx)
add_subdirectory(resource)
add_subdirectory(engine)
