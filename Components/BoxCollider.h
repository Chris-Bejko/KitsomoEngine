#pragma once

#include <string>
#include <vector>
#include "../Component.h"
#include <SFML/Graphics.hpp>

class BoxCollider : public Component
{
public:
	BoxCollider();

	BoxCollider(std::string tag, bool isTrigger = false);

	BoxCollider(const std::string tag, sf::FloatRect hitbox, bool isTrigger = false);

	virtual ~BoxCollider() = default;

	bool Init() override;

	void Serialize();

	std::vector<SerializableVariable> *GetSerializedFields() override final;

	void InitSerializedFields(ReadableSerializableVariableMap map);

	void SetUpColliderVisuals();

	void draw() override final;

	void update(float dt) override final;

	std::string GetCollisionTag();

	sf::FloatRect GetRect();

	inline bool IsTrigger()
	{
		return isTrigger;
	}
	void DrawEditorButton() override;
	bool IsInEditMode() { return editMode; }

private:
	std::vector<SerializableVariable> serializables;
	friend class Collision;
	float offset_x, offset_y;
	std::string collisionTag;
	sf::FloatRect hitbox;
	bool configuredHitbox;
	bool isTrigger;

	sf::RectangleShape colliderVisual;
	bool editMode = false;
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

