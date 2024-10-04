#ifndef GAME_GFX_SHADER_HPP
#define GAME_GFX_SHADER_HPP

#include "render_backend.hpp"

namespace Engine {

class Renderer;

struct Shader {
    friend Renderer;
private:
    GLuint program = 0;
};

} // namespace Engine

#endif
