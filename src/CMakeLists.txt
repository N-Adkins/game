target_sources(game PRIVATE
    main.cpp
    logging.cpp
    pch.cpp
)

target_precompile_headers(game PRIVATE pch.hpp)

add_subdirectory(gfx)
add_subdirectory(math)
add_subdirectory(resource)
