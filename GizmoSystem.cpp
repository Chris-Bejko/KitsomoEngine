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
#include "../Components/Canvas.h"
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

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        mode = GizmoMode::Move;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        mode = GizmoMode::Rotate;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
        mode = GizmoMode::Scale;
    snapEnabled = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl);

    bool isUI = selectedEntity->HasComponent<UIRect>();

    // Use SCREEN space for UI, WORLD space for everything else
    sf::Vector2f mousePos = isUI
                                ? (sf::Vector2f)sf::Mouse::getPosition(Engine::get().GetWindow())
                                : GetMouseWorld();

    sf::Vector2f entityCenter;
    if (isUI)
    {
        auto &ui = selectedEntity->GetComponent<UIRect>();
        auto rect = ui.GetScreenRect();
        entityCenter = {rect.left + rect.width * 0.5f, rect.top + rect.height * 0.5f};
    }
    else
    {
        Vector2F wp = selectedEntity->transform->GetWorldPosition();
        entityCenter = {wp.x, wp.y};
    }

    // Start drag
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && activeDrag == GizmoAxis::None)
    {
        GizmoAxis hovered = GetHoveredAxis(mousePos, entityCenter);
        if (hovered != GizmoAxis::None)
        {
            activeDrag = hovered;
            dragStartMouse = mousePos;
            dragStartPosition = isUI
                                    ? Vector2F(selectedEntity->GetComponent<UIRect>().anchorOffset.x,
                                               selectedEntity->GetComponent<UIRect>().anchorOffset.y)
                                    : selectedEntity->transform->position;
            dragStartRotation = selectedEntity->transform->rotation;
            dragStartScale = isUI
                                 ? Vector2F(selectedEntity->GetComponent<UIRect>().sizeDelta.x,
                                            selectedEntity->GetComponent<UIRect>().sizeDelta.y)
                                 : selectedEntity->transform->scale;
        }
    }

    // Release - register command
    if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) && activeDrag != GizmoAxis::None)
    {
        if (mode == GizmoMode::Move)
        {
            Vector2F cur = selectedEntity->transform->position;
            if (cur.x != dragStartPosition.x || cur.y != dragStartPosition.y)
                CommandHistory::get().Execute(std::make_unique<MoveEntityCommand>(
                    selectedEntity, dragStartPosition, cur));
        }
        if (mode == GizmoMode::Rotate)
        {
            float cur = selectedEntity->transform->rotation;
            if (cur != dragStartRotation)
                CommandHistory::get().Execute(std::make_unique<RotateEntityCommand>(
                    selectedEntity, dragStartRotation, cur));
        }
        if (mode == GizmoMode::Scale)
        {
            Vector2F cur = selectedEntity->transform->scale;
            if (cur.x != dragStartScale.x || cur.y != dragStartScale.y)
                CommandHistory::get().Execute(std::make_unique<ScaleEntityCommand>(
                    selectedEntity, dragStartScale, cur));
        }
        activeDrag = GizmoAxis::None;
    }

    if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
        activeDrag = GizmoAxis::None;

    if (activeDrag != GizmoAxis::None)
    {
        switch (mode)
        {
        case GizmoMode::Move:
            UpdateMove(mousePos, entityCenter);
            break;
        case GizmoMode::Rotate:
            UpdateRotate(mousePos, entityCenter);
            break;
        case GizmoMode::Scale:
            UpdateScale(mousePos, entityCenter);
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

void GizmoSystem::UpdateMove(sf::Vector2f mousePos, sf::Vector2f entityCenter)
{
    if (selectedEntity->HasComponent<Canvas>())
        return;

    sf::Vector2f delta = mousePos - dragStartMouse;

    if (selectedEntity->HasComponent<UIRect>())
    {
        auto &ui = selectedEntity->GetComponent<UIRect>();
        LOG_DEBUG("Before set - anchorOffset: ", ui.anchorOffset.x, ", ", ui.anchorOffset.y,
                  " dragStart: ", dragStartPosition.x, ", ", dragStartPosition.y,
                  " delta: ", delta.x, ", ", delta.y);
        if (activeDrag == GizmoAxis::X || activeDrag == GizmoAxis::XY)
            ui.anchorOffset.x = dragStartPosition.x + delta.x;
        if (activeDrag == GizmoAxis::Y || activeDrag == GizmoAxis::XY)
            ui.anchorOffset.y = dragStartPosition.y + delta.y;

        if (snapEnabled)
        {
            ui.anchorOffset.x = Snap(ui.anchorOffset.x, snapSize);
            ui.anchorOffset.y = Snap(ui.anchorOffset.y, snapSize);
        }
        LOG_DEBUG("After set - anchorOffset: ", ui.anchorOffset.x, ", ", ui.anchorOffset.y);
        return;
    }

    float newX = dragStartPosition.x;
    float newY = dragStartPosition.y;
    if (activeDrag == GizmoAxis::X || activeDrag == GizmoAxis::XY)
        newX += delta.x;
    if (activeDrag == GizmoAxis::Y || activeDrag == GizmoAxis::XY)
        newY += delta.y;
    if (snapEnabled)
    {
        newX = Snap(newX, snapSize);
        newY = Snap(newY, snapSize);
    }
    selectedEntity->transform->position = Vector2F(newX, newY);
}

void GizmoSystem::UpdateRotate(sf::Vector2f mouseWorld, sf::Vector2f entityWorld)
{
    if (selectedEntity->HasComponent<Canvas>())
        return; // Canvas cant be manipulated
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
    if (selectedEntity->HasComponent<Canvas>())
        return;

    sf::Vector2f delta = mouseWorld - dragStartMouse;

    if (selectedEntity->HasComponent<UIRect>())
    {
        auto &ui = selectedEntity->GetComponent<UIRect>();
        float scaleSensitivity = 1.0f; // screen space so 1:1

        if (activeDrag == GizmoAxis::X)
            ui.sizeDelta.x = std::max(1.f, dragStartScale.x + delta.x * scaleSensitivity);
        else if (activeDrag == GizmoAxis::Y)
            ui.sizeDelta.y = std::max(1.f, dragStartScale.y - delta.y * scaleSensitivity);
        else if (activeDrag == GizmoAxis::XY)
        {
            float uniform = (delta.x - delta.y) * scaleSensitivity;
            ui.sizeDelta.x = std::max(1.f, dragStartScale.x + uniform);
            ui.sizeDelta.y = std::max(1.f, dragStartScale.y + uniform);
        }

        if (snapEnabled)
        {
            ui.sizeDelta.x = Snap(ui.sizeDelta.x, 10.f);
            ui.sizeDelta.y = Snap(ui.sizeDelta.y, 10.f);
        }
        return;
    }

    float scaleSensitivity = 0.01f;

    if (activeDrag == GizmoAxis::X)
    {
        selectedEntity->transform->scale.x = std::max(
            0.01f,
            dragStartScale.x + delta.x * scaleSensitivity);
    }
    else if (activeDrag == GizmoAxis::Y)
    {
        selectedEntity->transform->scale.y = std::max(
            0.01f,
            dragStartScale.y - delta.y * scaleSensitivity);
    }
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

    bool isUI = selectedEntity->HasComponent<UIRect>();

    if (isUI)
    {
        // Switch to screen space view
        sf::View prevView = Engine::get().GetWindow().getView();
        Engine::get().GetWindow().setView(Engine::get().GetWindow().getDefaultView());

        auto &ui = selectedEntity->GetComponent<UIRect>();
        auto rect = ui.GetScreenRect();
        sf::Vector2f entityCenter(rect.left + rect.width * 0.5f,
                                  rect.top + rect.height * 0.5f);

        // Draw UIRect outline
        sf::RectangleShape outline(sf::Vector2f(rect.width, rect.height));
        outline.setPosition(rect.left, rect.top);
        outline.setFillColor(sf::Color::Transparent);
        outline.setOutlineColor(sf::Color(0, 200, 255, 200));
        outline.setOutlineThickness(1.f);
        Engine::get().GetWindow().draw(outline);

        // Draw normal gizmo arrows in screen space
        switch (mode)
        {
        case GizmoMode::Move:
            DrawMove(entityCenter);
            break;
        case GizmoMode::Rotate:
            DrawRotate(entityCenter);
            break;
        case GizmoMode::Scale:
            DrawScale(entityCenter);
            break;
        }

        Engine::get().GetWindow().setView(prevView);
        return;
    }

    // Normal world space
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