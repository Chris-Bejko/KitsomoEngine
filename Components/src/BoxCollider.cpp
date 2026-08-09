#include "BoxCollider.h"
#include "Engine.h"
#include "Sprite.h"
#include "CollisionSystem.h"
#include "ComponentRegistry.h"

REGISTER_SERIALIZABLE_COMPONENT(BoxCollider, true)

BoxCollider::BoxCollider()
{
    configuredHitbox = true;
    hitbox = sf::FloatRect(0, 0, 0, 0);
    collisionTag = "Enter tag";
    isTrigger = false;
}
BoxCollider::BoxCollider(std::string tag, bool isTrigger)
{
    collisionTag = tag;
    configuredHitbox = false;
    this->isTrigger = isTrigger;
}

BoxCollider::BoxCollider(const std::string tag, sf::FloatRect hitbox, bool isTrigger)
{
    collisionTag = tag;
    this->hitbox = hitbox;
    configuredHitbox = true;
}

bool BoxCollider::Init()
{
    Collider::Init();
    if (entity->HasComponent<Sprite>())
    {
        if (!configuredHitbox)
        {
            hitbox = entity->GetComponent<Sprite>().GetGlobalBounds();
        }
    }

    SetUpColliderVisuals();
    Field("hitbox.top", hitbox.top);
    Field("hitbox.left", hitbox.left);
    Field("hitbox.width", hitbox.width);
    Field("hitbox.height", hitbox.height);
    return true;
}

void BoxCollider::SetUpColliderVisuals()
{
    colliderVisual.setFillColor(sf::Color::Transparent);
    colliderVisual.setOutlineColor(sf::Color::Green);
    colliderVisual.setOutlineThickness(1);
    // colliderVisual.setPosition(sf::Vector2f(transform->position.x, transform->position.y));
    colliderVisual.setSize(sf::Vector2f(hitbox.width * entity->transform->scale.x, hitbox.height * entity->transform->scale.y));
    // colliderVisual.setOrigin(sprite->GetOrigin());
}

void BoxCollider::updateEngine(float dt)
{
    // colliderVisual.setPosition(sprite->GetPosition());
    // colliderVisual.setOrigin(sprite->GetOrigin());
    if (entity->HasComponent<Sprite>())
        colliderVisual.setPosition(GetRect().getPosition());

    UpdateEditMode();
}

std::string BoxCollider::GetCollisionTag()
{
    return collisionTag;
}

sf::FloatRect BoxCollider::GetRect()
{
    return entity->GetComponent<Sprite>().TranslateHitbox(hitbox);
}

void BoxCollider::draw()
{
    if (!Engine::get().isEngine)
        return;
    if (!editMode)
        return; // only draw in edit mode

    colliderVisual.setRotation(entity->transform->rotation);
    Engine::get().GetWindow().draw(colliderVisual);
    DrawHandles();
}

void BoxCollider::DrawHandles()
{
    sf::FloatRect rect = GetRect();
    float hw = 6.f; // handle size

    // 8 handle positions
    std::vector<sf::Vector2f> handles = {
        {rect.left, rect.top},                                // TopLeft
        {rect.left + rect.width / 2, rect.top},               // Top
        {rect.left + rect.width, rect.top},                   // TopRight
        {rect.left + rect.width, rect.top + rect.height / 2}, // Right
        {rect.left + rect.width, rect.top + rect.height},     // BottomRight
        {rect.left + rect.width / 2, rect.top + rect.height}, // Bottom
        {rect.left, rect.top + rect.height},                  // BottomLeft
        {rect.left, rect.top + rect.height / 2},              // Left
    };

    for (auto &pos : handles)
    {
        sf::RectangleShape handle(sf::Vector2f(hw, hw));
        handle.setOrigin(hw / 2, hw / 2);
        handle.setPosition(pos);
        handle.setFillColor(sf::Color::White);
        handle.setOutlineColor(sf::Color::Green);
        handle.setOutlineThickness(1);
        Engine::get().GetWindow().draw(handle);
    }
}

BoxCollider::DragHandle BoxCollider::GetHoveredHandle(sf::Vector2f mousePos)
{
    sf::FloatRect rect = GetRect();
    float hw = 8.f; // slightly larger hit area than visual
    // LOG_DEBUG("GetHoveredHandle - mouse: ", mousePos.x, ",", mousePos.y,
    //   " rect: ", rect.left, ",", rect.top, " ", rect.width, "x", rect.height);
    auto nearBy = [](float a, float b)
    { return std::abs(a - b) < 8.f; };
    bool onLeft = nearBy(mousePos.x, rect.left);
    bool onRight = nearBy(mousePos.x, rect.left + rect.width);
    bool onTop = nearBy(mousePos.y, rect.top);
    bool onBottom = nearBy(mousePos.y, rect.top + rect.height);

    if (onTop && onLeft)
        return DragHandle::TopLeft;
    if (onTop && onRight)
        return DragHandle::TopRight;
    if (onBottom && onLeft)
        return DragHandle::BottomLeft;
    if (onBottom && onRight)
        return DragHandle::BottomRight;
    if (onTop)
        return DragHandle::Top;
    if (onBottom)
        return DragHandle::Bottom;
    if (onLeft)
        return DragHandle::Left;
    if (onRight)
        return DragHandle::Right;

    return DragHandle::None;
}

void BoxCollider::UpdateEditMode()
{
    if (!editMode)
        return;
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    auto mousePixel = sf::Mouse::getPosition(Engine::get().GetWindow());
    auto mouseWorld = Engine::get().GetWindow().mapPixelToCoords(mousePixel);

    // Start drag
    if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && activeDrag == DragHandle::None)
    {
        DragHandle hovered = GetHoveredHandle(mouseWorld);
        if (hovered != DragHandle::None)
        {
            activeDrag = hovered;
            dragStartMouse = mouseWorld;
            dragStartHitbox = hitbox;
        }
    }

    // Release drag
    if (!sf::Mouse::isButtonPressed(sf::Mouse::Right))
    {
        activeDrag = DragHandle::None;
    }

    // Apply drag
    if (activeDrag != DragHandle::None)
    {
        sf::Vector2f delta = mouseWorld - dragStartMouse;

        float scaleX = entity->transform->scale.x;
        float scaleY = entity->transform->scale.y;

        if (scaleX != 0)
            delta.x /= scaleX;
        if (scaleY != 0)
            delta.y /= scaleY;

        hitbox = dragStartHitbox;
        switch (activeDrag)
        {
        case DragHandle::Top:
            hitbox.top += delta.y;
            hitbox.height -= delta.y;
            break;
        case DragHandle::Bottom:
            hitbox.height += delta.y;
            break;
        case DragHandle::Left:
            hitbox.left += delta.x;
            hitbox.width -= delta.x;
            break;
        case DragHandle::Right:
            hitbox.width += delta.x;
            break;
        case DragHandle::TopLeft:
            hitbox.top += delta.y;
            hitbox.height -= delta.y;
            hitbox.left += delta.x;
            hitbox.width -= delta.x;
            break;
        case DragHandle::TopRight:
            hitbox.top += delta.y;
            hitbox.height -= delta.y;
            hitbox.width += delta.x;
            break;
        case DragHandle::BottomLeft:
            hitbox.height += delta.y;
            hitbox.left += delta.x;
            hitbox.width -= delta.x;
            break;
        case DragHandle::BottomRight:
            hitbox.height += delta.y;
            hitbox.width += delta.x;
            break;
        default:
            break;
        }

        // Prevent negative size
        if (hitbox.width < 1.f)
            hitbox.width = 1.f;
        if (hitbox.height < 1.f)
            hitbox.height = 1.f;

        // Update visual
        SetUpColliderVisuals();
        Serialize();
    }
}

void BoxCollider::DrawEditorButton()
{
    if (editMode)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
        if (ImGui::Button("Stop Editing", ImVec2(-1, 24)))
            editMode = false;
        ImGui::PopStyleColor();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.7f, 1.0f));
        if (ImGui::Button("Edit Collider", ImVec2(-1, 24)))
            editMode = true;
        ImGui::PopStyleColor();
    }
}

bool BoxCollider::Intersects(Collider &other)
{
    if (other.GetType() == ColliderType::Box)
        return CollisionSystem::get().AABB(GetBounds(), other.GetBounds());

    if (other.GetType() == ColliderType::Circle)
        return CollisionSystem::get().AABBvsCircle(
            GetBounds(),
            other.GetBounds().getPosition() + sf::Vector2f(other.GetBounds().width / 2.f, other.GetBounds().height / 2.f),
            other.GetBounds().width / 2.f);

    return false;
}

sf::FloatRect BoxCollider::GetBounds()
{
    return GetRect();
}

void BoxCollider::DrawDebug()
{
    colliderVisual.setRotation(entity->transform->rotation);
    Engine::get().GetWindow().draw(colliderVisual);
}