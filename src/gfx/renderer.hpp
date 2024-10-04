#ifndef GAME_GFX_RENDERER_HPP
#define GAME_GFX_RENDERER_HPP

#include "../math/vec3.hpp"
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

    void setViewport(size_t width, size_t height);
    void setBackgroundColor(const Vec3& color);

    void startFrame();
    void endFrame();

private:
    Vec3 background_color = Vec3(0.2f, 0.3f, 0.3f);
};

} // namespace Engine

#endif
