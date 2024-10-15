project(engine
    LANGUAGES C
)

add_library(engine SHARED)

target_include_directories(engine PRIVATE src)

target_compile_options(engine PRIVATE
    -Wall -Wformat -Wextra -Werror -fdiagnostics-color=always
)

target_compile_definitions(engine PRIVATE LENGINE_BUILD)

find_package(X11 REQUIRED)
target_include_directories(engine PRIVATE ${X11_INCLUDE_DIR})
target_link_libraries(engine PRIVATE ${X11_LIBRARIES})

add_subdirectory(src)
