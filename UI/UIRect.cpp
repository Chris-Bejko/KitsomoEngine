#include "UIRect.h"
#include "../Components/Canvas.h"
#include "../Engine.h"
#include "../Components/Transform.h"
#include "../GizmoSystem.h"
bool UIRect::Init()
{
    Serialize();
    if (entity->HasComponent<Transform>())
        entity->GetComponent<Transform>().isUITransform = true;
    return true;
}

Canvas *UIRect::GetCanvas()
{
    if (cachedCanvas != nullptr)
        return cachedCanvas;

    // Walk up entity hierarchy to find Canvas
    Entity *current = entity->GetParent();
    while (current != nullptr)
    {
        if (current->HasComponent<Canvas>())
        {
            cachedCanvas = &current->GetComponent<Canvas>();
            return cachedCanvas;
        }
        current = current->GetParent();
    }
    return nullptr;
}

void UIRect::SyncToTransform()
{
    if (!entity->HasComponent<Transform>())
        return;

    auto screenPos = GetScreenPosition();
    entity->transform->position = Vector2F(screenPos.x, screenPos.y);
}
sf::Vector2f UIRect::ResolveAnchor(AnchorPreset anchor, sf::Vector2f screenSize)
{
    switch (anchor)
    {
    case AnchorPreset::TopLeft:
        return {0, 0};
    case AnchorPreset::TopCenter:
        return {screenSize.x * 0.5f, 0};
    case AnchorPreset::TopRight:
        return {screenSize.x, 0};
    case AnchorPreset::MiddleLeft:
        return {0, screenSize.y * 0.5f};
    case AnchorPreset::MiddleCenter:
        return {screenSize.x * 0.5f, screenSize.y * 0.5f};
    case AnchorPreset::MiddleRight:
        return {screenSize.x, screenSize.y * 0.5f};
    case AnchorPreset::BottomLeft:
        return {0, screenSize.y};
    case AnchorPreset::BottomCenter:
        return {screenSize.x * 0.5f, screenSize.y};
    case AnchorPreset::BottomRight:
        return {screenSize.x, screenSize.y};
    default:
        return {screenSize.x * 0.5f, screenSize.y * 0.5f};
    }
}

sf::Vector2f UIRect::GetScreenPosition()
{
    Canvas *canvas = GetCanvas();
    sf::Vector2f screenSize;

    if (canvas && canvas->GetRenderMode() == CanvasRenderMode::ScreenSpace)
    {
        auto windowSize = Engine::get().GetWindow().getSize();
        screenSize = {(float)windowSize.x, (float)windowSize.y};
    }
    else
    {
        // World space - use world coordinates
        return {entity->transform->GetWorldPosition().x,
                entity->transform->GetWorldPosition().y};
    }

    sf::Vector2f anchorPos = ResolveAnchor(anchor, screenSize);
    sf::Vector2f pivotOffset = {sizeDelta.x * pivot.x, sizeDelta.y * pivot.y};

    return anchorPos + anchorOffset - pivotOffset;
}

sf::FloatRect UIRect::GetScreenRect()
{
    sf::Vector2f pos = GetScreenPosition();
    return sf::FloatRect(pos.x, pos.y, sizeDelta.x, sizeDelta.y);
}

void UIRect::Serialize()
{
    serializables.clear();
    serializables.push_back({"anchorOffset.x", &anchorOffset.x, float_Type});
    serializables.push_back({"anchorOffset.y", &anchorOffset.y, float_Type});
    serializables.push_back({"sizeDelta.x", &sizeDelta.x, float_Type});
    serializables.push_back({"sizeDelta.y", &sizeDelta.y, float_Type});
    serializables.push_back({"pivot.x", &pivot.x, float_Type});
    serializables.push_back({"pivot.y", &pivot.y, float_Type});
}

void UIRect::InitSerializedFields(ReadableSerializableVariableMap map)
{
    for (auto const &[key, value] : map.floatFields)
    {
        if (key == "anchorOffset.x")
            anchorOffset.x = value;
        if (key == "anchorOffset.y")
            anchorOffset.y = value;
        if (key == "sizeDelta.x")
            sizeDelta.x = value;
        if (key == "sizeDelta.y")
            sizeDelta.y = value;
        if (key == "pivot.x")
            pivot.x = value;
        if (key == "pivot.y")
            pivot.y = value;
    }
}

void UIRect::updateEngine(float dt)
{
    if (GizmoSystem::get().GetSelectedEntity() == entity &&
        GizmoSystem::get().IsGizmoDragging())
    {
        // Convert transform position back to anchored position
        Canvas *canvas = GetCanvas();
        if (canvas)
        {
            auto windowSize = Engine::get().GetWindow().getSize();
            sf::Vector2f anchorPos = ResolveAnchor(anchor,
                                                   sf::Vector2f(windowSize.x, windowSize.y));
            anchorOffset.x = entity->transform->position.x - anchorPos.x;
            anchorOffset.y = entity->transform->position.y - anchorPos.y;
        }
    }
    else
    {

        SyncToTransform();
    }
}

void UIRect::update(float dt)
{
    SyncToTransform();
}