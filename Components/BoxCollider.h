#pragma once

#include <string>
#include <vector>
#include "../Component.h"
#include <SFML/Graphics.hpp>
#include "../Collision/Collider.h"

class BoxCollider : public Collider
{
public:
	BoxCollider();

	BoxCollider(std::string tag, bool isTrigger = false);

	BoxCollider(const std::string tag, sf::FloatRect hitbox, bool isTrigger = false);

	virtual ~BoxCollider() = default;

	bool Init() override;

    bool Intersects(Collider& other) override;
    sf::FloatRect GetBounds() override;
    void DrawDebug() override;
    ColliderType GetType() override { return ColliderType::Box; }

	void Serialize() override final;

	std::vector<SerializableVariable> *GetSerializedFields() override final;

	void InitSerializedFields(ReadableSerializableVariableMap map) override final;

	void SetUpColliderVisuals();

	void draw() override final;

	void updateEngine(float dt) override final;

	std::string GetCollisionTag();

	sf::FloatRect GetRect();

	void DrawEditorButton() override;

private:
	std::vector<SerializableVariable> serializables;
	friend class Collision;
	float offset_x, offset_y;
	sf::FloatRect hitbox;
	bool configuredHitbox;
	sf::RectangleShape colliderVisual;
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

	DragHandle activeDrag = DragHandle::None;
	sf::Vector2f dragStartMouse;
	sf::FloatRect dragStartHitbox;
	void UpdateEditMode();
	DragHandle GetHoveredHandle(sf::Vector2f mousePos);
	void DrawHandles();

};

