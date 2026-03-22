#pragma once
#include "../Collision/Collider.h"

class CircleCollider : public Collider
{
public:
    CircleCollider();
    CircleCollider(std::string tag, float radius, bool isTrigger = false);

    bool Init() override;
    void update(float dt) override;
    void updateEngine(float dt) override;
    void draw() override;

    bool Intersects(Collider& other) override;
    sf::FloatRect GetBounds() override;
    void DrawDebug() override;
    ColliderType GetType() override { return ColliderType::Circle; }

    float GetRadius() { return radius; }
    sf::Vector2f GetCenter();

private:
    float radius = 50.f;
    sf::CircleShape colliderVisual;
    bool configuredRadius = false;

    // Edit mode drag
    void UpdateEditMode();
    sf::Vector2f dragStartMouse;
    float dragStartRadius = 0.f;
    bool draggingRadius = false;
};