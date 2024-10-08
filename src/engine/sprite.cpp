#include <pch.hpp>

#include "sprite.hpp"
#include "../gfx/buffer.hpp"
#include "../gfx/opengl.hpp"

namespace Engine {

Sprite::Sprite(SpriteManager* manager, SpriteId id)
    : manager(manager), id(id) 
{
    manager->updated_sprites.insert(id);
}

void Sprite::destroy()
{
    if (manager != nullptr) {
        manager->free_ids.insert(id);
    }
}

void Sprite::setPosition(Vec2 position)
{
    this->position = position;
    manager->updated_sprites.insert(id);
}

Vec2 Sprite::getPosition() const
{
    return position;
}

void Sprite::setScale(float scale)
{
    this->scale = scale;
    manager->updated_sprites.insert(id);
}

float Sprite::getScale() const
{
    return scale;
}

SpriteManager::SpriteManager(const Shader& shader)
    : shader(shader)
{
    VertexBufferLayout layout = shader.getUniformLayout();
    vert_array.addBuffer(vert_buffer, layout);
}

Sprite& SpriteManager::createSprite()
{
    SpriteId id;
    if (!free_ids.empty()) {
        id = *free_ids.begin();
        free_ids.erase(id);
    } else {
        id = sprites.size();
        sprites.push_back(Sprite(this, id));
    }

    return sprites[id];
}

void SpriteManager::render()
{
    std::vector<SpriteVertexData> vert_data;
    
    vert_array.bind();
    
    if (!updated_sprites.empty()) {
        for (size_t i = 0; i < sprites.size(); i++) {
            const auto& sprite = sprites[i];
            if (free_ids.contains(sprite.id)) {
                continue;
            }

            auto data = SpriteVertexData {
                .index = 0,
                .position = sprite.getPosition(),
                .scale = sprite.getScale(),
            };

            for (unsigned int j = 0; j < 3; j++) {
                data.index = j;
                vert_data.push_back(data);
            }
        }

        Log::debug("Buffering {} verts for sprites", vert_data.size());
        vert_array.bind();
        vert_buffer.buffer(
            static_cast<const void*>(&vert_data[0]),
            vert_data.size() * sizeof(SpriteVertexData)
        );
    }
    
    vert_array.unbind();
    shader.use();
    vert_array.bind();

    if (!vert_buffer.isEmpty()) {
        OPENGL_CALL(glDrawArrays(
            GL_TRIANGLES,
            0,
            static_cast<GLsizei>((sprites.size() - free_ids.size()) * 3)
        ));
    }
        
    updated_sprites.clear();
}

} // namespace Engine
