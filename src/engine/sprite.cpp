#include <pch.hpp>

#include "sprite.hpp"
#include "gfx/buffer.hpp"

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
    this->vert_data.position = position;
    manager->updated_sprites.insert(id);
}

Vec2 Sprite::getPosition() const
{
    return vert_data.position;
}

void Sprite::setScale(float scale)
{
    this->vert_data.scale = scale;
    manager->updated_sprites.insert(id);
}

Vec2 Sprite::getScale() const
{
    return vert_data.scale;
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
    }

    sprites.push_back(Sprite(this, id));

    return sprites[id];
}

void SpriteManager::render()
{
    std::vector<SpriteVertexData> vert_data;
    for (size_t i = 0; i < sprites.size(); i++) {
        if (free_ids.contains(i)) {
            continue;
        }
        const auto& sprite = sprites[i];
        vert_data.push_back(sprite.vert_data);
    }
    
    if (!updated_sprites.empty()) {
        vert_buffer.buffer(
            static_cast<const void*>(&vert_data[0]),
            vert_data.size() * sizeof(SpriteVertexData)
        );
    }
    
    shader.use();
    vert_array.bind();
    glDrawElements(
        GL_TRIANGLES, 
        static_cast<GLsizei>(vert_data.size() * 6), 
        GL_UNSIGNED_INT, 
        0
    );

    updated_sprites.clear();
}

} // namespace Engine
