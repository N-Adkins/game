#pragma once

#include "../gfx/array.hpp"
#include "../resource/shader.hpp"
#include "../constructors.hpp"
#include "../platform.hpp"
#include <unordered_set>

namespace sol {
    class state;
}

namespace Engine {

using SpriteId = size_t;

class SpriteManager;

GAME_PACKED_STRUCT(SpriteVertexData, {
    GLuint index = 0;
    float x = 0.f;
    float y = 0.f;
    float scale = 1.f;
});

class Sprite {
private:
    Sprite(SpriteManager* manager, SpriteId id);

public:
    DELETE_COPY(Sprite);
    DEFAULT_MOVE(Sprite);

    void destroy();
    void setPosition(glm::vec2 position);
    glm::vec2 getPosition() const;
    void setScale(float scale);
    float getScale() const;
    SpriteId getId() const;

private:
    SpriteManager* manager;
    SpriteId id = 0;

    friend SpriteManager;
};

struct SpriteData {
    SpriteData(SpriteId id) 
        : id(id) {}
    glm::vec2 position = glm::vec2(0.f, 0.f);
    float scale = 1.f;
    SpriteId id;
};

class SpriteManager {
public:
    SpriteManager(const Shader& shader);

    Sprite createSprite();
    void render();

private:
    std::vector<SpriteData> sprite_data;
    std::unordered_set<SpriteId> free_ids;
    std::unordered_set<SpriteId> updated_sprites;

    VertexArray vert_array;
    VertexBuffer vert_buffer;
    const Shader& shader;

    friend Sprite;
};

} // namespace Engine
