#pragma once

#include <string>
#include "../Entity.h"
#include "../Component.h"
#include "SFML/Graphics.hpp"
#include "Sprite.h"

class BoxCollider : public Component
{
public:

	BoxCollider(std::string tag, bool isTrigger = false)
	{
		collisionTag = tag;
		configuredHitbox = false;
		this->isTrigger = isTrigger;
	}

	BoxCollider(const std::string tag, sf::FloatRect hitbox, bool isTrigger = false)
	{
		collisionTag = tag;
		this->hitbox = hitbox;
		configuredHitbox = true;

	}
	virtual ~BoxCollider() = default;

	bool Init() override 
	{
		if (entity->HasComponent<Sprite>())
		{
			sprite = &entity->GetComponent<Sprite>();
			if (!configuredHitbox)
			{
				hitbox = sprite->GetGlobalBounds();
			}

		}

		SetUpColliderVisuals();
		//transform = &entity->GetComponent<Transform>();
		Serialize();

		return true;
	}
	void Serialize()
	{
		serializables.clear();
		serializables.push_back({ "collider Top", &hitbox.top, Float_Type });
		serializables.push_back({ "collider Left", &hitbox.left, Float_Type });
		serializables.push_back({ "collider Width", &hitbox.width, Float_Type });
		serializables.push_back({ "collider Height", &hitbox.height, Float_Type });
		serializables.push_back({ "Tag", &collisionTag, Char_Type });
	}

	std::vector<SerializableVariable> *GetSerializedFields() override final
	{
		return &serializables;
	}
	void SetUpColliderVisuals()
	{
		colliderVisual.setFillColor(sf::Color::Transparent);
		colliderVisual.setOutlineColor(sf::Color::Green);
		colliderVisual.setOutlineThickness(1);
		//colliderVisual.setPosition(sf::Vector2f(transform->position.x, transform->position.y));
		colliderVisual.setSize(sf::Vector2f(hitbox.width * entity->transform->scale.x, hitbox.height * entity->transform->scale.y));
		//colliderVisual.setOrigin(sprite->GetOrigin());
	}

	void draw() override final
	{
		colliderVisual.setRotation(entity->transform->rotation);
		Engine::get().GetWindow().draw(colliderVisual);
	}

	void update(float dt) override final
	{
		//colliderVisual.setPosition(sprite->GetPosition());
		//colliderVisual.setOrigin(sprite->GetOrigin());
		colliderVisual.setPosition(GetRect().getPosition());
	}

	
	std::string GetCollisionTag() 
	{
		return collisionTag;
	}

	sf::FloatRect GetRect()
	{
		return sprite->TranslateHitbox(hitbox);
	}

	sf::Sprite GetSprite()
	{
		return sprite->GetSprite();
	}

	bool IsTrigger()
	{
		return isTrigger;
	}
private:
	std::vector<SerializableVariable> serializables;
	friend class Collision;
	float offset_x, offset_y;
	std::string collisionTag;
	Sprite* sprite = nullptr;
	sf::FloatRect hitbox;
	bool configuredHitbox;
	bool isTrigger;

	sf::RectangleShape colliderVisual;
};