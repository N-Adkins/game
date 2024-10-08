#pragma once

#include "../gfx/array.hpp"
#include "../resource/shader.hpp"
#include "../math/vec2.hpp"
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
    Vec2 position = Vec2(0.f, 0.f);
    float scale = 1.f;
});

class Sprite {
private:
    Sprite(SpriteManager* manager, SpriteId id);

public:
    DELETE_COPY(Sprite);
    DEFAULT_MOVE(Sprite);

    void destroy();
    void setPosition(Vec2 position);
    Vec2 getPosition() const;
    void setScale(float scale);
    float getScale() const;
    SpriteId getId() const;

private:
    Vec2 position = Vec2(0.f, 0.f);
    float scale = 1.f;
    SpriteManager* manager;
    SpriteId id = 0;

    friend SpriteManager;
};

class SpriteManager {
public:
    SpriteManager(const Shader& shader);

    Sprite& createSprite();
    void render();

private:
    std::vector<Sprite> sprites;
    std::unordered_set<SpriteId> free_ids;
    std::unordered_set<SpriteId> updated_sprites;

    VertexArray vert_array;
    VertexBuffer vert_buffer;
    const Shader& shader;

    friend Sprite;
};

} // namespace Engine
