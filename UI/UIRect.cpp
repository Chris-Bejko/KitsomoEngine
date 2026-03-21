#include "UIRect.h"
#include "../Components/Canvas.h"
#include "../Engine.h"
#include "../Components/Transform.h"
#include "../GizmoSystem.h"
#include "Logger.h"
bool UIRect::Init()
{
    if (entity->HasComponent<Transform>())
        entity->GetComponent<Transform>().isUITransform = true;

    Field("anchorOffset.x", anchorOffset.x);
    Field("anchorOffset.y", anchorOffset.y);
    Field("sizeDelta.x", sizeDelta.x);
    Field("sizeDelta.y", sizeDelta.y);
    Field("pivot.x", pivot.x);
    Field("pivot.y", pivot.y);
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
        return {entity->transform->GetWorldPosition().x,
                entity->transform->GetWorldPosition().y};
    }

    // Check if parent entity has a UIRect - position relative to it
    if (entity->GetParent() != nullptr &&
        entity->GetParent()->HasComponent<UIRect>())
    {
        auto &parentRect = entity->GetParent()->GetComponent<UIRect>();
        sf::FloatRect parentScreenRect = parentRect.GetScreenRect();

        // Anchor relative to parent rect instead of screen
        sf::Vector2f parentAnchorPos = ResolveAnchor(anchor,
                                                     sf::Vector2f(parentScreenRect.width, parentScreenRect.height));

        sf::Vector2f pivotOffset = {sizeDelta.x * pivot.x, sizeDelta.y * pivot.y};

        return sf::Vector2f(
            parentScreenRect.left + parentAnchorPos.x + anchorOffset.x - pivotOffset.x,
            parentScreenRect.top + parentAnchorPos.y + anchorOffset.y - pivotOffset.y);
    }

    // Normal screen space positioning
    sf::Vector2f anchorPos = ResolveAnchor(anchor, screenSize);
    sf::Vector2f pivotOffset = {sizeDelta.x * pivot.x, sizeDelta.y * pivot.y};
    return anchorPos + anchorOffset - pivotOffset;
}

sf::FloatRect UIRect::GetScreenRect()
{
    sf::Vector2f pos = GetScreenPosition();
    return sf::FloatRect(pos.x, pos.y, sizeDelta.x, sizeDelta.y);
}

void UIRect::updateEngine(float dt)
{
    SyncToTransform();
}

void UIRect::update(float dt)
{
    SyncToTransform();
}