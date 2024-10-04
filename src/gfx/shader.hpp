#pragma once

#include "../constructors.hpp"

namespace Engine {

class Renderer;

struct Shader {
    Shader() = default;
    ~Shader();
    DELETE_COPY(Shader);
    DEFAULT_MOVE(Shader);

    friend Renderer;
private:
    unsigned int program = 0;
};

} // namespace Engine
