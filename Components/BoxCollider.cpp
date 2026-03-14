#include "BoxCollider.h"
#include "../Engine.h"
#include "Sprite.h"
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
	if (entity->HasComponent<Sprite>())
	{
		if (!configuredHitbox)
		{
			hitbox = entity->GetComponent<Sprite>().GetGlobalBounds();
		}

	}

	SetUpColliderVisuals();
	//transform = &entity->GetComponent<Transform>();
	Serialize();

	return true;
}

void BoxCollider::Serialize()
{
	serializables.clear();
	serializables.push_back({ "hitbox.top", &hitbox.top, float_Type });
	serializables.push_back({ "hitbox.left", &hitbox.left, float_Type });
	serializables.push_back({ "hitbox.width", &hitbox.width, float_Type });
	serializables.push_back({ "hitbox.height", &hitbox.height, float_Type });
	serializables.push_back({ "collisionTag", &collisionTag, char_Type });
	serializables.push_back({ "isTrigger", &isTrigger, bool_Type });
}

std::vector<SerializableVariable>* BoxCollider::GetSerializedFields()
{
	return &serializables;
}

void BoxCollider::InitSerializedFields(ReadableSerializableVariableMap map)
{
	for (auto const& [key, value] : map.floatFields)
	{
		if (key == "hitbox.top")
		{
			hitbox.top = value;
		}
		if (key == "hitbox.left")
		{
			hitbox.left = value;
		}
		if (key == "hitbox.width")
		{
			hitbox.width = value;
		}
		if (key == "hitbox.height")
		{
			hitbox.height = value;
		}
	}
	for (auto const& [key, value] : map.stringFields)
	{
		if (key == "collisionTag")
		{
			collisionTag = value;
		}
	}

	for (auto const& [key, value] : map.boolFields)
	{
		if (key == "isTrigger")
		{
			isTrigger = value;
		}
	}
}

void BoxCollider::SetUpColliderVisuals()
{
	colliderVisual.setFillColor(sf::Color::Transparent);
	colliderVisual.setOutlineColor(sf::Color::Green);
	colliderVisual.setOutlineThickness(1);
	//colliderVisual.setPosition(sf::Vector2f(transform->position.x, transform->position.y));
	colliderVisual.setSize(sf::Vector2f(hitbox.width * entity->transform->scale.x, hitbox.height * entity->transform->scale.y));
	//colliderVisual.setOrigin(sprite->GetOrigin());
}


void BoxCollider::update(float dt)
{
	//colliderVisual.setPosition(sprite->GetPosition());
	//colliderVisual.setOrigin(sprite->GetOrigin());
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
    if (!Engine::get().isEngine) return;
    if (!editMode) return;  // only draw in edit mode

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
        { rect.left,                    rect.top },                    // TopLeft
        { rect.left + rect.width / 2,   rect.top },                    // Top
        { rect.left + rect.width,       rect.top },                    // TopRight
        { rect.left + rect.width,       rect.top + rect.height / 2 },  // Right
        { rect.left + rect.width,       rect.top + rect.height },       // BottomRight
        { rect.left + rect.width / 2,   rect.top + rect.height },       // Bottom
        { rect.left,                    rect.top + rect.height },       // BottomLeft
        { rect.left,                    rect.top + rect.height / 2 },   // Left
    };

    for (auto& pos : handles)
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

    auto near = [](float a, float b) { return std::abs(a - b) < 8.f; };
    bool onLeft   = near(mousePos.x, rect.left);
    bool onRight  = near(mousePos.x, rect.left + rect.width);
    bool onTop    = near(mousePos.y, rect.top);
    bool onBottom = near(mousePos.y, rect.top + rect.height);

    if (onTop    && onLeft)  return DragHandle::TopLeft;
    if (onTop    && onRight) return DragHandle::TopRight;
    if (onBottom && onLeft)  return DragHandle::BottomLeft;
    if (onBottom && onRight) return DragHandle::BottomRight;
    if (onTop)               return DragHandle::Top;
    if (onBottom)            return DragHandle::Bottom;
    if (onLeft)              return DragHandle::Left;
    if (onRight)             return DragHandle::Right;

    return DragHandle::None;
}

void BoxCollider::UpdateEditMode()
{
    if (!editMode) return;
    if (ImGui::GetIO().WantCaptureMouse) return;

    auto mousePixel = sf::Mouse::getPosition(Engine::get().GetWindow());
    auto mouseWorld = Engine::get().GetWindow().mapPixelToCoords(mousePixel);

    // Start drag
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && activeDrag == DragHandle::None)
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
    if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        activeDrag = DragHandle::None;
    }

    // Apply drag
    if (activeDrag != DragHandle::None)
    {
        sf::Vector2f delta = mouseWorld - dragStartMouse;

        hitbox = dragStartHitbox;

        switch (activeDrag)
        {
        case DragHandle::Top:
            hitbox.top    += delta.y;
            hitbox.height -= delta.y;
            break;
        case DragHandle::Bottom:
            hitbox.height += delta.y;
            break;
        case DragHandle::Left:
            hitbox.left  += delta.x;
            hitbox.width -= delta.x;
            break;
        case DragHandle::Right:
            hitbox.width += delta.x;
            break;
        case DragHandle::TopLeft:
            hitbox.top    += delta.y;
            hitbox.height -= delta.y;
            hitbox.left   += delta.x;
            hitbox.width  -= delta.x;
            break;
        case DragHandle::TopRight:
            hitbox.top    += delta.y;
            hitbox.height -= delta.y;
            hitbox.width  += delta.x;
            break;
        case DragHandle::BottomLeft:
            hitbox.height += delta.y;
            hitbox.left   += delta.x;
            hitbox.width  -= delta.x;
            break;
        case DragHandle::BottomRight:
            hitbox.height += delta.y;
            hitbox.width  += delta.x;
            break;
        default: break;
        }

        // Prevent negative size
        if (hitbox.width  < 1.f) hitbox.width  = 1.f;
        if (hitbox.height < 1.f) hitbox.height = 1.f;

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