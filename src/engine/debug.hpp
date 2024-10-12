#pragma once

namespace Engine {

class DebugContext {
public:
    void tryRender(float delta_time);
    void toggle();
private:
    void render(float delta_time);
    bool enabled = false;
    bool wireframe = false;
};

} // namespace Engine
