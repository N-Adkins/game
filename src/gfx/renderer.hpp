#ifndef GAME_GFX_RENDERER_HPP
#define GAME_GFX_RENDERER_HPP

#include "shader.hpp"
#include <cstddef>
#include <string>

namespace Engine {

class Renderer {
public:
    Shader loadShader(
        const std::string& name,
        const unsigned char* vert_bytes, 
        size_t vert_len, 
        const unsigned char* frag_bytes, 
        size_t frag_len
    );
};

} // namespace Engine

#endif
