#pragma once

#include <string>
#include "../Entity.h"
#include "../Component.h"
#include "SFML/Graphics.hpp"
#include "Sprite.h"

class BoxCollider2D : public Component
{
public:
	bool isTrigger;

	BoxCollider2D(std::string tag, bool isTrigger = false)
	{
		collisionTag = tag;
		configuredHitbox = false;
		this->isTrigger = isTrigger;
	}

	BoxCollider2D(std::string tag, sf::FloatRect hitbox, bool isTrigger = false)
	{
		collisionTag = tag;
		this->hitbox = hitbox;
		this->isTrigger = isTrigger;
		configuredHitbox = true;
	}
	~BoxCollider2D() = default;

	bool Init() override final
	{
		transform = &entity->GetComponent<Transform>();
		if (entity->HasComponent<Sprite>())
		{
			sprite = &entity->GetComponent<Sprite>();
			if (!configuredHitbox)
			{
				hitbox = sprite->GetGlobalBounds();

				std::cout << hitbox.height << std::endl;
				std::cout << hitbox.width << std::endl;
				std::cout << hitbox.left << std::endl;
				std::cout << hitbox.top << std::endl;
			}

		}
		return true;
	}

	void draw() override final
	{
	}

	void update() override final
	{
	}

	std::string GetCollisionTag() const
	{
		return collisionTag;
	}

	sf::FloatRect GetRect()
	{
		return sprite->TranslateHitbox(hitbox);
		//return sprite->GetGlobalBounds();
	}


private:
	friend class Collision;
	std::string collisionTag = "";
	Transform* transform = nullptr;
	Sprite* sprite = nullptr;
	sf::FloatRect hitbox;
	bool configuredHitbox;
};