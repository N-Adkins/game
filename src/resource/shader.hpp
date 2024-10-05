#pragma once

#include "resource.hpp"
#include "../constructors.hpp"
#include <string_view>

namespace Engine {

class Shader : public Resource {
public:
    ~Shader();
    DELETE_COPY(Shader);
    DEFAULT_MOVE(Shader);

    constexpr static std::string_view RESOURCE_NAME = "Shader";

private:
    Shader(
        const std::string& name,
        const unsigned char* vert_bytes, 
        size_t vert_len, 
        const unsigned char* frag_bytes, 
        size_t frag_len
    );
    unsigned int program = 0;
};

}
