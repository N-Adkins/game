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
        manager = nullptr;
    }
}

void Sprite::setPosition(Vec2 position)
{
    manager->sprite_data[getId()].position = position;
    manager->updated_sprites.insert(id);
}

Vec2 Sprite::getPosition() const
{
    return manager->sprite_data[getId()].position;
}

void Sprite::setScale(float scale)
{
    manager->sprite_data[getId()].scale = scale;
    manager->updated_sprites.insert(id);
}

float Sprite::getScale() const
{
    return manager->sprite_data[getId()].scale;
}

SpriteId Sprite::getId() const
{
    return id;
}

SpriteManager::SpriteManager(const Shader& shader)
    : shader(shader)
{
    VertexBufferLayout layout = shader.getUniformLayout();
    vert_array.addBuffer(vert_buffer, layout);
}

Sprite SpriteManager::createSprite()
{
    SpriteId id;
    if (!free_ids.empty()) {
        id = *free_ids.begin();
        free_ids.erase(id);
        sprite_data[id] = SpriteData(id);
    } else {
        id = sprite_data.size();
        sprite_data.push_back(SpriteData(id));
    }

    return Sprite { this, id };
}

void SpriteManager::render()
{
    std::vector<SpriteVertexData> vert_data;
    
    vert_array.bind();
    
    if (!updated_sprites.empty()) {
        for (size_t i = 0; i < sprite_data.size(); i++) {
            const auto& sprite = sprite_data[i];
            if (free_ids.contains(sprite.id)) {
                continue;
            }

            auto data = SpriteVertexData {
                .index = 0,
                .position = sprite.position,
                .scale = sprite.scale,
            };

            for (unsigned int j = 0; j < 6; j++) {
                data.index = j;
                vert_data.push_back(data);
            }
        }

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
            static_cast<GLsizei>((sprite_data.size() - free_ids.size()) * 6)
        ));
    }
        
    updated_sprites.clear();
}

} // namespace Engine
