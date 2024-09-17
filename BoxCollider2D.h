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
		std::cout << isTrigger << std::endl;
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
			}

		}

		SetUpColliderVisuals();


		return true;
	}

	void SetUpColliderVisuals()
	{
		colliderVisual.setFillColor(sf::Color::Transparent);
		colliderVisual.setOutlineColor(sf::Color::Green);
		colliderVisual.setOutlineThickness(1);
		//colliderVisual.setPosition(sf::Vector2f(transform->position.x, transform->position.y));
		colliderVisual.setSize(sf::Vector2f(hitbox.width * transform->scale.x, hitbox.height * transform->scale.y));
		//colliderVisual.setOrigin(sprite->GetOrigin());
	}

	void draw() override final
	{
		colliderVisual.setRotation(transform->rotation);
		Engine::get().GetWindow().draw(colliderVisual);
	}

	void update() override final
	{
		//colliderVisual.setPosition(sprite->GetPosition());
		//colliderVisual.setOrigin(sprite->GetOrigin());
		colliderVisual.setPosition(GetRect().getPosition());
	}

	std::string GetCollisionTag() const
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
private:
	friend class Collision;
	float offset_x, offset_y;
	std::string collisionTag = "";
	Transform* transform = nullptr;
	Sprite* sprite = nullptr;
	sf::FloatRect hitbox;
	bool configuredHitbox;

	sf::RectangleShape colliderVisual;
};