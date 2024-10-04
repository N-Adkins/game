#pragma once

namespace Engine {

class Renderer;

struct Shader {
    friend Renderer;
private:
    unsigned int program = 0;
};

} // namespace Engine
