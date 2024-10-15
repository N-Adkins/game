project(engine
    LANGUAGES C
)

add_library(engine SHARED)

target_include_directories(engine PRIVATE src)
target_compile_definitions(engine PRIVATE LENGINE_BUILD)

target_compile_options(engine PRIVATE
    -Wall -Wformat -Wextra -Werror -fdiagnostics-color=always
)

if (NOT WIN32)
    find_package(X11 REQUIRED)
    target_link_libraries(engine PRIVATE ${X11_LIBRARIES})
endif()

add_subdirectory(src)
