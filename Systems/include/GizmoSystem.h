#pragma once
#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "Vector2.h"
#include "BoxCollider.h"

enum class GizmoMode
{
    Move,
    Rotate,
    Scale
};

enum class GizmoAxis
{
    None,
    X,
    Y,
    XY, // for scale uniform
    Rotate
};

class GizmoSystem
{
public:
    static GizmoSystem &get()
    {
        static GizmoSystem instance;
        return instance;
    }

    void Update(float dt);
    void Draw();
    void Clear() { selectedEntity = nullptr; activeDrag = GizmoAxis::None; }

    void SetSelectedEntity(Entity *entity) { selectedEntity = entity; }
    void SetMode(GizmoMode mode) { this->mode = mode; }
    GizmoMode GetMode() { return mode; }

    // Grid snap
    bool snapEnabled = false;
    float snapSize = 16.f;
    bool IsGizmoDragging() { return activeDrag != GizmoAxis::None; }
    Entity* GetSelectedEntity() { return selectedEntity; }
private:
    GizmoSystem() = default;

    Entity *selectedEntity = nullptr;
    GizmoMode mode = GizmoMode::Move;
    GizmoAxis activeDrag = GizmoAxis::None;

    sf::Vector2f dragStartMouse;
    Vector2F dragStartPosition;
    float dragStartRotation = 0.f;
    Vector2F dragStartScale;

    float arrowLength = 80.f;
    float arrowThickness = 3.f;
    float handleSize = 10.f;
    float rotateRadius = 60.f;

    sf::Vector2f GetMouseWorld();
    GizmoAxis GetHoveredAxis(sf::Vector2f mouseWorld, sf::Vector2f entityWorld);
    float Snap(float value, float snapSize);

    void UpdateMove(sf::Vector2f mouseWorld, sf::Vector2f entityWorld);
    void UpdateRotate(sf::Vector2f mouseWorld, sf::Vector2f entityWorld);
    void UpdateScale(sf::Vector2f mouseWorld, sf::Vector2f entityWorld);

    void DrawMove(sf::Vector2f entityWorld);
    void DrawRotate(sf::Vector2f entityWorld);
    void DrawScale(sf::Vector2f entityWorld);

    void DrawArrow(sf::Vector2f from, sf::Vector2f to, sf::Color color, float thickness);
	enum class DragHandle
	{
		None,
		Top,
		Bottom,
		Left,
		Right,
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight,
	};
    GizmoSystem::DragHandle activeDragHandle = GizmoSystem::DragHandle::None;
    GizmoSystem::DragHandle GetHoveredHandle(sf::Vector2f pos, sf::FloatRect rect);
};