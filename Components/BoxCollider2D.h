#pragma once

#include <string>
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
		serializables.push_back({ "hitbox.top", &hitbox.top, float_Type });
		serializables.push_back({ "hitbox.left", &hitbox.left, float_Type });
		serializables.push_back({ "hitbox.width", &hitbox.width, float_Type });
		serializables.push_back({ "hitbox.height", &hitbox.height, float_Type });
		serializables.push_back({ "collisionTag", &collisionTag, char_Type });
		serializables.push_back({ "isTrigger", &isTrigger, bool_Type });
	}

	std::vector<SerializableVariable> *GetSerializedFields() override final
	{
		return &serializables;
	}

	void InitSerializedFields(ReadableSerializableVariableMap map)
	{
		for(auto const& [key, value] : map.floatFields)
		{
			if(key == "hitbox.top")
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
		for(auto const& [key, value] : map.stringFields)
		{
			if(key == "collisionTag")
			{
				collisionTag = value;
			}
		}

		for(auto const& [key, value] : map.boolFields)
		{
			if(key == "isTrigger")
			{
				isTrigger = value;
			}
		}
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
		if (!Engine::get().isEngine)
			return;

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