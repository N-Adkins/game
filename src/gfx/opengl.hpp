#pragma once

#include "../logging.hpp"
#include <cstdlib>

#define OPENGL_CALL(call) \
    call; \
    if (glGetError() != GL_NO_ERROR) { \
        Log::error("OpenGL call failed: {}", #call); \
        std::exit(EXIT_FAILURE); \
    } \
    do {} while(0)
