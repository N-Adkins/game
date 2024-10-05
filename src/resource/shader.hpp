#pragma once

#include "resource.hpp"
#include "../constructors.hpp"
#include <filesystem>
#include <string_view>

namespace Engine {

class ResourceManager;

class Shader : public Resource {
public:
    ~Shader();
    DELETE_COPY(Shader);
    DEFAULT_MOVE(Shader);

    constexpr static std::string_view RESOURCE_NAME = "Shader";

private: 
    friend ResourceManager;

    struct ShaderData {
        std::string vert;
        std::string frag;
    };

    Shader(const std::filesystem::path& path);
    std::optional<ShaderData> preProcessShader(const std::string& file);

    unsigned int program = 0;
};

}
