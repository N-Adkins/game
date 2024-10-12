#include <pch.hpp>

#include "debug.hpp"
#include "imgui.h"

namespace Engine {

void DebugContext::tryRender(float delta_time)
{
    if (enabled) {
        render(delta_time);
    }
}

void DebugContext::toggle()
{
    enabled = !enabled;
}

void DebugContext::render(float delta_time)
{
    ImGui::Begin("Debug");
    
    if (wireframe) {
        if (ImGui::Button("Disable Wireframe")) {
            wireframe = false;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    } else {
        if (ImGui::Button("Enable Wireframe")) {
            wireframe = true;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
    }

    ImGui::Text("FPS: %.1f", 1.f / delta_time);

    ImGui::End();
}

} // namespace Engine
