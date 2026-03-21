#include "Canvas.h"
#include "../Engine.h"
#include "imgui.h"

bool Canvas::Init()
{
    Field("renderMode", renderModeString);
    Field("referenceResolution.x", referenceResolution.x);
    Field("referenceResolution.y", referenceResolution.y);
    return true;
}

void Canvas::draw() {}
void Canvas::update(float dt) {}
void Canvas::updateEngine(float dt) {}

void Canvas::DrawEditorButton()
{
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Render Mode");
    if (ImGui::RadioButton("Screen Space", renderMode == CanvasRenderMode::ScreenSpace))
    {
        renderMode = CanvasRenderMode::ScreenSpace;
        renderModeString = "ScreenSpace";
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("World Space", renderMode == CanvasRenderMode::WorldSpace))
    {
        renderMode = CanvasRenderMode::WorldSpace;
        renderModeString = "WorldSpace";
    }
}