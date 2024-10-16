project(engine
    LANGUAGES C
)

add_library(engine SHARED)

target_include_directories(engine PRIVATE src)
target_compile_definitions(engine PRIVATE LENGINE_BUILD)

if (NOT WIN32)
    find_package(X11 REQUIRED)
    target_link_libraries(engine PRIVATE ${X11_LIBRARIES})

    target_compile_options(engine PRIVATE
        -Wall -Wformat -Wextra -Werror -fdiagnostics-color=always 
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message("Building in debug mode, enabling sanitizer")
        target_compile_options(engine PRIVATE -fsanitize=address,undefined)
        target_link_options(engine PRIVATE -fsanitize=address,undefined)
    endif()

endif()

add_subdirectory(src)
