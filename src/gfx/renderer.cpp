#include <pch.hpp>

#include "renderer.hpp"
#include "shader.hpp"

namespace Engine {

Shader Renderer::loadShader(
    const std::string& name,
    const unsigned char* vert_bytes, 
    size_t vert_len, 
    const unsigned char* frag_bytes, 
    size_t frag_len)
{
    Log::debug("Attempting to load shader \"{}\"", name);

#if defined(GAME_RENDER_BACKEND_OPENGL)

    const GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    const GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderBinary(1, &vert, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, vert_bytes, static_cast<GLsizei>(vert_len));
    glShaderBinary(1, &frag, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, frag_bytes, static_cast<GLsizei>(frag_len));

    glSpecializeShader(vert, "main", 0, 0, 0);
    glSpecializeShader(frag, "main", 0, 0, 0);

    int compiled = 0;
    char log[512];
    glGetShaderiv(vert, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        glGetShaderInfoLog(vert, 512, nullptr, log);
        std::string_view safe_log = log;
        Log::error("Failed to compile vertex shader for \"{}\" -> \"{}\"", name, safe_log);
        Shader incomplete_shader;
        incomplete_shader.program = 0;
        return incomplete_shader;
    }
    
    glGetShaderiv(frag, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        glGetShaderInfoLog(frag, 512, nullptr, log);
        std::string_view safe_log = log;
        Log::error("Failed to compile fragment shader for \"{}\" -> \"{}\"", name, safe_log);
        glDeleteShader(vert);
        Shader incomplete_shader;
        incomplete_shader.program = 0;
        return incomplete_shader;
    }
    
    const GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &compiled);
    if (!compiled) {
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::string_view safe_log = log;
        Log::error("Failed to link shader program for \"{}\" -> \"{}\"", name, safe_log);
        Shader incomplete_shader;
        incomplete_shader.program = 0;
        return incomplete_shader;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    Log::info("Successfully loaded shader \"{}\"", name);

    Shader shader;
    shader.program = program;
    return shader;

#endif
}

void Renderer::setViewport(size_t width, size_t height)
{
#if defined(GAME_RENDER_BACKEND_OPENGL)
    glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(height));
#endif
}

void Renderer::setBackgroundColor(const Vec3& color)
{
    background_color = color;
}

void Renderer::startFrame()
{

}

void Renderer::endFrame()
{
#if defined(GAME_RENDER_BACKEND_OPENGL)
    glClearColor(background_color.getX(), background_color.getY(), background_color.getZ(), 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
#endif
}

} // namespace Engine
