#pragma once

#include "resource.hpp"
#include "../constructors.hpp"
#include "../gfx/buffer.hpp"
#include "../math/vec2.hpp"
#include "../math/vec3.hpp"
#include "../math/mat4.hpp"
#include <filesystem>
#include <string_view>

namespace sol {
    class state;
}

namespace Engine {

class ResourceManager;

class Shader : public Resource {
public:
    ~Shader();
    DELETE_COPY(Shader);
    DEFAULT_MOVE(Shader);

    constexpr static std::string_view RESOURCE_NAME = "Shader";
    
    void use() const;
    void setUniform(const std::string& name, float value) const;
    void setUniform(const std::string& name, Vec2 value) const;
    void setUniform(const std::string& name, const Vec3& value) const;
    void setUniform(const std::string& name, const Mat4& value) const;

    VertexBufferLayout getUniformLayout() const;

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
