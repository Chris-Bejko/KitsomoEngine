#include "CircleCollider.h"
#include "Engine.h"
#include "Sprite.h"
#include "CollisionSystem.h"
#include "ComponentRegistry.h"

REGISTER_SERIALIZABLE_COMPONENT(CircleCollider, true)

CircleCollider::CircleCollider()
{
    collisionTag = "default";
    isTrigger = false;
}

CircleCollider::CircleCollider(std::string tag, float radius, bool isTrigger)
{
    collisionTag = tag;
    this->radius = radius;
    this->isTrigger = isTrigger;
    configuredRadius = true;
}

bool CircleCollider::Init()
{
    Collider::Init();
    if (!configuredRadius && entity->HasComponent<Sprite>())
    {
        auto bounds = entity->GetComponent<Sprite>().GetGlobalBounds();
        radius = std::min(bounds.width, bounds.height) / 2.f;
    }

    colliderVisual.setRadius(radius);
    colliderVisual.setOrigin(radius, radius);
    colliderVisual.setFillColor(sf::Color::Transparent);
    colliderVisual.setOutlineColor(sf::Color(0, 255, 128, 255));
    colliderVisual.setOutlineThickness(1.f);
    Field("radius", radius);
    return true;
}

sf::Vector2f CircleCollider::GetCenter()
{
    return sf::Vector2f(
        entity->transform->position.x,
        entity->transform->position.y);
}

sf::FloatRect CircleCollider::GetBounds()
{
    return sf::FloatRect(
        GetCenter().x - radius,
        GetCenter().y - radius,
        radius * 2.f,
        radius * 2.f);
}

bool CircleCollider::Intersects(Collider &other)
{
    if (other.GetType() == ColliderType::Circle)
    {
        auto &circle = static_cast<CircleCollider &>(other);
        sf::Vector2f diff = GetCenter() - circle.GetCenter();
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        return dist < (radius + circle.GetRadius());
    }
    if (other.GetType() == ColliderType::Box)
    {
        return CollisionSystem::get().AABBvsCircle(other.GetBounds(), GetCenter(), radius);
    }
    return false;
}

void CircleCollider::DrawDebug()
{
    colliderVisual.setPosition(GetCenter());
    Engine::get().GetWindow().draw(colliderVisual);
}

void CircleCollider::draw()
{
    if (!Engine::get().isEngine)
        return;
    if (!editMode)
        return;
    DrawDebug();
    UpdateEditMode();
}

void CircleCollider::update(float dt)
{
    if (editMode)
    {
        colliderVisual.setRadius(radius);
        colliderVisual.setOrigin(radius, radius);
    }
}
void CircleCollider::updateEngine(float dt)
{
    if (editMode)
    {
        colliderVisual.setRadius(radius);
        colliderVisual.setOrigin(radius, radius);
    }
}
void CircleCollider::UpdateEditMode()
{
    if (!editMode)
        return;
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    auto mousePixel = sf::Mouse::getPosition(Engine::get().GetWindow());
    auto mouseWorld = Engine::get().GetWindow().mapPixelToCoords(mousePixel);

    sf::Vector2f center = GetCenter();
    sf::Vector2f diff = mouseWorld - center;
    float distToMouse = std::sqrt(diff.x * diff.x + diff.y * diff.y);

    // Drag the edge to resize
    bool nearEdge = std::abs(distToMouse - radius) < 8.f;

    if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && !draggingRadius)
    {
        if (nearEdge)
        {
            draggingRadius = true;
            dragStartMouse = mouseWorld;
            dragStartRadius = radius;
        }
    }

    if (!sf::Mouse::isButtonPressed(sf::Mouse::Right))
        draggingRadius = false;

    if (draggingRadius)
    {
        sf::Vector2f d = mouseWorld - center;
        radius = std::max(1.f, std::sqrt(d.x * d.x + d.y * d.y));
        colliderVisual.setRadius(radius);
        colliderVisual.setOrigin(radius, radius);
        Serialize();
    }
}