#include "GizmoSystem.h"
#include "../Engine.h"
#include "../Logger.h"
#include "imgui.h"
#include <cmath>
#include "Components/Transform.h"
#include "Commands/MoveEntityCommand.h"
#include "Commands/ScaleEntityCommand.h"
#include "Commands/RotateEntityCommand.h"
#include "Commands/CommandHistory.h"
#include "../UI/UIRect.h"
sf::Vector2f GizmoSystem::GetMouseWorld()
{
    return Engine::get().GetWindow().mapPixelToCoords(
        sf::Mouse::getPosition(Engine::get().GetWindow()));
}

float GizmoSystem::Snap(float value, float size)
{
    return std::round(value / size) * size;
}

void GizmoSystem::Update(float dt)
{
    if (!selectedEntity)
        return;
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    // Keyboard shortcuts
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        mode = GizmoMode::Move;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        mode = GizmoMode::Rotate;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
        mode = GizmoMode::Scale;

    // Toggle snap with Ctrl
    snapEnabled = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl);

    Vector2F worldPosV = selectedEntity->transform->GetWorldPosition();
    sf::Vector2f entityWorld(worldPosV.x, worldPosV.y);
    sf::Vector2f mouseWorld = GetMouseWorld();

    // Start drag
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && activeDrag == GizmoAxis::None)
    {
        GizmoAxis hovered = GetHoveredAxis(mouseWorld, entityWorld);
        if (hovered != GizmoAxis::None)
        {
            activeDrag = hovered;
            dragStartMouse = mouseWorld;
            dragStartPosition = selectedEntity->transform->position;
            dragStartRotation = selectedEntity->transform->rotation;
            dragStartScale = selectedEntity->transform->scale;
        }
    }

    if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) && activeDrag != GizmoAxis::None)
    {
        if (mode == GizmoMode::Move)
        {
            Vector2F currentPos = selectedEntity->transform->position;
            if (currentPos.x != dragStartPosition.x || currentPos.y != dragStartPosition.y)
            {
                auto cmd = std::make_unique<MoveEntityCommand>(
                    selectedEntity, dragStartPosition, currentPos);
                CommandHistory::get().Execute(std::move(cmd));
            }
        }
        if (mode == GizmoMode::Rotate)
        {
            float currentRot = selectedEntity->transform->rotation;
            if (currentRot != dragStartRotation)
            {
                auto cmd = std::make_unique<RotateEntityCommand>(
                    selectedEntity, dragStartRotation, currentRot);
                CommandHistory::get().Execute(std::move(cmd));
            }
        }
        if (mode == GizmoMode::Scale)
        {
            Vector2F currentScale = selectedEntity->transform->scale;
            if (currentScale.x != dragStartScale.x || currentScale.y != dragStartScale.y)
            {
                auto cmd = std::make_unique<ScaleEntityCommand>(
                    selectedEntity, dragStartScale, currentScale);
                CommandHistory::get().Execute(std::move(cmd));
            }
        }
        activeDrag = GizmoAxis::None;
    }
    // Release drag
    if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
        activeDrag = GizmoAxis::None;

    // Apply drag
    if (activeDrag != GizmoAxis::None)
    {
        switch (mode)
        {
        case GizmoMode::Move:
            UpdateMove(mouseWorld, entityWorld);
            break;
        case GizmoMode::Rotate:
            UpdateRotate(mouseWorld, entityWorld);
            break;
        case GizmoMode::Scale:
            UpdateScale(mouseWorld, entityWorld);
            break;
        }
    }
}

GizmoAxis GizmoSystem::GetHoveredAxis(sf::Vector2f mouseWorld, sf::Vector2f entityWorld)
{
    auto isNear = [](sf::Vector2f a, sf::Vector2f b, float radius)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return (dx * dx + dy * dy) < (radius * radius);
    };

    switch (mode)
    {
    case GizmoMode::Move:
    {
        // X axis handle
        sf::Vector2f xHandle(entityWorld.x + arrowLength, entityWorld.y);
        if (isNear(mouseWorld, xHandle, handleSize * 1.5f))
            return GizmoAxis::X;

        // Y axis handle
        sf::Vector2f yHandle(entityWorld.x, entityWorld.y - arrowLength);
        if (isNear(mouseWorld, yHandle, handleSize * 1.5f))
            return GizmoAxis::Y;

        // Center XY handle
        if (isNear(mouseWorld, entityWorld, handleSize))
            return GizmoAxis::XY;
        break;
    }
    case GizmoMode::Rotate:
    {
        float dx = mouseWorld.x - entityWorld.x;
        float dy = mouseWorld.y - entityWorld.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (std::abs(dist - rotateRadius) < 8.f)
            return GizmoAxis::Rotate;
        break;
    }
    case GizmoMode::Scale:
    {
        sf::Vector2f xHandle(entityWorld.x + arrowLength, entityWorld.y);
        sf::Vector2f yHandle(entityWorld.x, entityWorld.y - arrowLength);
        sf::Vector2f xyHandle(entityWorld.x + arrowLength * 0.5f,
                              entityWorld.y - arrowLength * 0.5f);

        if (isNear(mouseWorld, xHandle, handleSize * 1.5f))
            return GizmoAxis::X;
        if (isNear(mouseWorld, yHandle, handleSize * 1.5f))
            return GizmoAxis::Y;
        if (isNear(mouseWorld, xyHandle, handleSize * 1.5f))
            return GizmoAxis::XY;
        break;
    }
    }
    return GizmoAxis::None;
}

void GizmoSystem::UpdateMove(sf::Vector2f mouseWorld, sf::Vector2f entityWorld)
{
    sf::Vector2f delta = mouseWorld - dragStartMouse;

    float newX = dragStartPosition.x;
    float newY = dragStartPosition.y;

    if (activeDrag == GizmoAxis::X || activeDrag == GizmoAxis::XY)
        newX = dragStartPosition.x + delta.x;
    if (activeDrag == GizmoAxis::Y || activeDrag == GizmoAxis::XY)
        newY = dragStartPosition.y + delta.y;

    if (snapEnabled)
    {
        newX = Snap(newX, snapSize);
        newY = Snap(newY, snapSize);
    }

    selectedEntity->transform->position = Vector2F(newX, newY);
}

void GizmoSystem::UpdateRotate(sf::Vector2f mouseWorld, sf::Vector2f entityWorld)
{
    float angle = atan2(mouseWorld.y - entityWorld.y,
                        mouseWorld.x - entityWorld.x) *
                  180.f / 3.14159f;

    float startAngle = atan2(dragStartMouse.y - entityWorld.y,
                             dragStartMouse.x - entityWorld.x) *
                       180.f / 3.14159f;

    float delta = angle - startAngle;
    float newRot = dragStartRotation + delta;

    if (snapEnabled)
        newRot = Snap(newRot, 15.f); // snap to 15 degree increments

    selectedEntity->transform->rotation = newRot;
}

void GizmoSystem::UpdateScale(sf::Vector2f mouseWorld, sf::Vector2f entityWorld)
{
    sf::Vector2f delta = mouseWorld - dragStartMouse;
    float scaleSensitivity = 0.01f;

    if (activeDrag == GizmoAxis::X)
        selectedEntity->transform->scale.x = std::max(0.01f,
                                                      dragStartScale.x + delta.x * scaleSensitivity);
    else if (activeDrag == GizmoAxis::Y)
        selectedEntity->transform->scale.y = std::max(0.01f,
                                                      dragStartScale.y - delta.y * scaleSensitivity);
    else if (activeDrag == GizmoAxis::XY)
    {
        float uniform = (delta.x - delta.y) * scaleSensitivity;
        selectedEntity->transform->scale.x = std::max(0.01f, dragStartScale.x + uniform);
        selectedEntity->transform->scale.y = std::max(0.01f, dragStartScale.y + uniform);
    }

    if (snapEnabled)
    {
        selectedEntity->transform->scale.x = Snap(selectedEntity->transform->scale.x, 0.1f);
        selectedEntity->transform->scale.y = Snap(selectedEntity->transform->scale.y, 0.1f);
    }
}

void GizmoSystem::DrawArrow(sf::Vector2f from, sf::Vector2f to, sf::Color color, float thickness)
{
    sf::Vector2f dir = to - from;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len == 0)
        return;
    sf::Vector2f norm = dir / len;
    sf::Vector2f perp(-norm.y, norm.x);

    // Line
    sf::VertexArray line(sf::Quads, 4);
    sf::Vector2f half = perp * (thickness / 2.f);
    line[0].position = from - half;
    line[1].position = from + half;
    line[2].position = to + half;
    line[3].position = to - half;
    for (int i = 0; i < 4; i++)
        line[i].color = color;
    Engine::get().GetWindow().draw(line);

    // Arrowhead
    float headSize = 12.f;
    sf::VertexArray head(sf::Triangles, 3);
    head[0].position = to + norm * headSize;
    head[1].position = to + perp * headSize * 0.5f - norm * headSize * 0.5f;
    head[2].position = to - perp * headSize * 0.5f - norm * headSize * 0.5f;
    for (int i = 0; i < 3; i++)
        head[i].color = color;
    Engine::get().GetWindow().draw(head);
}

void GizmoSystem::DrawMove(sf::Vector2f entityWorld)
{
    sf::Color xColor = (activeDrag == GizmoAxis::X) ? sf::Color::Yellow : sf::Color::Red;
    sf::Color yColor = (activeDrag == GizmoAxis::Y) ? sf::Color::Yellow : sf::Color::Green;
    sf::Color xyColor = (activeDrag == GizmoAxis::XY) ? sf::Color::Yellow : sf::Color::White;

    // X arrow (right)
    DrawArrow(entityWorld,
              sf::Vector2f(entityWorld.x + arrowLength, entityWorld.y),
              xColor, arrowThickness);

    // Y arrow (up)
    DrawArrow(entityWorld,
              sf::Vector2f(entityWorld.x, entityWorld.y - arrowLength),
              yColor, arrowThickness);

    // Center handle
    sf::CircleShape center(handleSize / 2.f);
    center.setOrigin(handleSize / 2.f, handleSize / 2.f);
    center.setPosition(entityWorld);
    center.setFillColor(xyColor);
    Engine::get().GetWindow().draw(center);
}

void GizmoSystem::DrawRotate(sf::Vector2f entityWorld)
{
    sf::Color color = (activeDrag == GizmoAxis::Rotate) ? sf::Color::Yellow : sf::Color(0, 200, 255);

    // Draw rotation circle
    int segments = 64;
    sf::VertexArray circle(sf::LineStrip, segments + 1);
    for (int i = 0; i <= segments; i++)
    {
        float angle = (float)i / segments * 2.f * 3.14159f;
        circle[i].position = sf::Vector2f(
            entityWorld.x + cos(angle) * rotateRadius,
            entityWorld.y + sin(angle) * rotateRadius);
        circle[i].color = color;
    }
    Engine::get().GetWindow().draw(circle);

    // Draw current rotation indicator line
    float rot = selectedEntity->transform->GetWorldRotation() * 3.14159f / 180.f;
    sf::Vector2f indicator(
        entityWorld.x + cos(rot) * rotateRadius,
        entityWorld.y + sin(rot) * rotateRadius);
    DrawArrow(entityWorld, indicator, sf::Color::Yellow, 2.f);
}

void GizmoSystem::DrawScale(sf::Vector2f entityWorld)
{
    sf::Color xColor = (activeDrag == GizmoAxis::X) ? sf::Color::Yellow : sf::Color::Red;
    sf::Color yColor = (activeDrag == GizmoAxis::Y) ? sf::Color::Yellow : sf::Color::Green;
    sf::Color xyColor = (activeDrag == GizmoAxis::XY) ? sf::Color::Yellow : sf::Color::White;

    sf::Vector2f xEnd(entityWorld.x + arrowLength, entityWorld.y);
    sf::Vector2f yEnd(entityWorld.x, entityWorld.y - arrowLength);
    sf::Vector2f xyEnd(entityWorld.x + arrowLength * 0.5f, entityWorld.y - arrowLength * 0.5f);

    // Lines
    DrawArrow(entityWorld, xEnd, xColor, arrowThickness);
    DrawArrow(entityWorld, yEnd, yColor, arrowThickness);

    // Square handles
    auto drawSquare = [&](sf::Vector2f pos, sf::Color color)
    {
        sf::RectangleShape sq(sf::Vector2f(handleSize, handleSize));
        sq.setOrigin(handleSize / 2.f, handleSize / 2.f);
        sq.setPosition(pos);
        sq.setFillColor(color);
        Engine::get().GetWindow().draw(sq);
    };

    drawSquare(xEnd, xColor);
    drawSquare(yEnd, yColor);
    drawSquare(xyEnd, xyColor);
}

void GizmoSystem::Draw()
{
    if (!selectedEntity)
        return;
    if (!Engine::get().isEngine)
        return;

    if (selectedEntity->HasComponent<UIRect>())
    {
        DrawUIRectGizmo();
        return;
    }

    Vector2F worldPosV = selectedEntity->transform->GetWorldPosition();
    sf::Vector2f entityWorld(worldPosV.x, worldPosV.y);

    switch (mode)
    {
    case GizmoMode::Move:
        DrawMove(entityWorld);
        break;
    case GizmoMode::Rotate:
        DrawRotate(entityWorld);
        break;
    case GizmoMode::Scale:
        DrawScale(entityWorld);
        break;
    }
}


void GizmoSystem::DrawUIRectGizmo()
{
    auto& rect = selectedEntity->GetComponent<UIRect>();
    sf::FloatRect screenRect = rect.GetScreenRect();

    // Switch to screen space view
    sf::View prevView = Engine::get().GetWindow().getView();
    Engine::get().GetWindow().setView(Engine::get().GetWindow().getDefaultView());

    // Draw rect outline
    sf::RectangleShape outline(sf::Vector2f(screenRect.width, screenRect.height));
    outline.setPosition(screenRect.left, screenRect.top);
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineColor(sf::Color(0, 200, 255, 200));
    outline.setOutlineThickness(1.f);
    Engine::get().GetWindow().draw(outline);

    // Draw corner/edge handles for resizing
    auto drawHandle = [&](sf::Vector2f pos) {
        sf::RectangleShape h(sf::Vector2f(8.f, 8.f));
        h.setOrigin(4.f, 4.f);
        h.setPosition(pos);
        h.setFillColor(sf::Color::White);
        h.setOutlineColor(sf::Color(0, 200, 255));
        h.setOutlineThickness(1.f);
        Engine::get().GetWindow().draw(h);
    };

    // 8 handles - corners and edges
    drawHandle({screenRect.left,                          screenRect.top});
    drawHandle({screenRect.left + screenRect.width / 2,   screenRect.top});
    drawHandle({screenRect.left + screenRect.width,        screenRect.top});
    drawHandle({screenRect.left + screenRect.width,        screenRect.top + screenRect.height / 2});
    drawHandle({screenRect.left + screenRect.width,        screenRect.top + screenRect.height});
    drawHandle({screenRect.left + screenRect.width / 2,   screenRect.top + screenRect.height});
    drawHandle({screenRect.left,                          screenRect.top + screenRect.height});
    drawHandle({screenRect.left,                          screenRect.top + screenRect.height / 2});

    Engine::get().GetWindow().setView(prevView);
}