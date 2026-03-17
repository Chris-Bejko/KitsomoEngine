#include "Canvas.h"
#include "../Engine.h"
#include "imgui.h"

bool Canvas::Init()
{
    Serialize();
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

void Canvas::Serialize()
{
    serializables.clear();
    serializables.push_back({"renderMode", &renderModeString, char_Type});
    serializables.push_back({"referenceResolution.x", &referenceResolution.x, float_Type});
    serializables.push_back({"referenceResolution.y", &referenceResolution.y, float_Type});
}

void Canvas::InitSerializedFields(ReadableSerializableVariableMap map)
{
    for (auto const& [key, value] : map.stringFields)
    {
        if (key == "renderMode")
        {
            renderModeString = value;
            renderMode = (value == "WorldSpace") ? 
                CanvasRenderMode::WorldSpace : CanvasRenderMode::ScreenSpace;
        }
    }
    for (auto const& [key, value] : map.floatFields)
    {
        if (key == "referenceResolution.x") referenceResolution.x = value;
        if (key == "referenceResolution.y") referenceResolution.y = value;
    }
}