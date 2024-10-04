#include <pch.hpp>

#include "shader.hpp"

namespace Engine {

Shader::~Shader()
{
#if defined(GAME_RENDER_BACKEND_OPENGL)
    if (program != 0) {
        glDeleteProgram(program);
    }
#endif
}

} // namespace Engine
