#include "Canvas.h"
#include "Engine.h"
#include "imgui.h"
#include "ComponentRegistry.h"

DECLARE_COMPONENT_RULES(Canvas, false)
REGISTER_COMPONENT(Canvas)
REGISTER_SERIALIZABLE_COMPONENT(Canvas)

bool Canvas::Init()
{
    Field("renderMode", renderModeString);
    Field("scaleMode", scaleModeString);
    Field("referenceResolution", referenceResolution);
    return true;
}

void Canvas::draw()
{
    if (!Engine::get().isEngine)
        return;
    if (renderMode != CanvasRenderMode::ScreenSpace)
        return;

    sf::View prevView = Engine::get().GetWindow().getView();
    Engine::get().GetWindow().setView(Engine::get().GetWindow().getDefaultView());

    auto windowSize = Engine::get().GetWindow().getSize();

    // Draw reference resolution outline
    float scaleX = windowSize.x / referenceResolution.x;
    float scaleY = windowSize.y / referenceResolution.y;
    float scale = std::min(scaleX, scaleY);

    float canvasW = referenceResolution.x * scale;
    float canvasH = referenceResolution.y * scale;
    float canvasX = (windowSize.x - canvasW) * 0.5f;
    float canvasY = (windowSize.y - canvasH) * 0.5f;

    sf::RectangleShape bounds(sf::Vector2f(canvasW, canvasH));
    bounds.setPosition(canvasX, canvasY);
    bounds.setFillColor(sf::Color::Transparent);
    bounds.setOutlineColor(sf::Color(100, 100, 255, 80));
    bounds.setOutlineThickness(1.f);
    Engine::get().GetWindow().draw(bounds);

    // Draw safe area guides
    sf::Color guideColor(100, 100, 255, 30);
    sf::VertexArray hLine(sf::Lines, 2);
    hLine[0].position = {0, windowSize.y * 0.5f};
    hLine[1].position = {(float)windowSize.x, windowSize.y * 0.5f};
    hLine[0].color = hLine[1].color = guideColor;
    Engine::get().GetWindow().draw(hLine);

    sf::VertexArray vLine(sf::Lines, 2);
    vLine[0].position = {windowSize.x * 0.5f, 0};
    vLine[1].position = {windowSize.x * 0.5f, (float)windowSize.y};
    vLine[0].color = vLine[1].color = guideColor;
    Engine::get().GetWindow().draw(vLine);

    Engine::get().GetWindow().setView(prevView);
}
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

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Scale Mode");
    if (ImGui::RadioButton("Constant Pixel Size", scaleMode == CanvasScaleMode::ConstantPixelSize))
    {
        scaleMode = CanvasScaleMode::ConstantPixelSize;
        scaleModeString = "ConstantPixelSize";
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale With Screen", scaleMode == CanvasScaleMode::ScaleWithScreenSize))
    {
        scaleMode = CanvasScaleMode::ScaleWithScreenSize;
        scaleModeString = "ScaleWithScreenSize";
    }

    if (scaleMode == CanvasScaleMode::ScaleWithScreenSize)
    {
        ImGui::Text("Reference: %.0f x %.0f", 
                    referenceResolution.x, referenceResolution.y);
        ImGui::Text("Scale Factor: %.3f", scaleFactor);
    }
}

void Canvas::UpdateScaleFactor()
{
    if (scaleMode == CanvasScaleMode::ConstantPixelSize)
    {
        scaleFactor = 1.0f;
        return;
    }
    auto windowSize = Engine::get().GetWindow().getSize();
    float scaleX = windowSize.x / referenceResolution.x;
    float scaleY = windowSize.y / referenceResolution.y;
    scaleFactor = std::min(scaleX, scaleY); // uniform scale
}