#pragma once

#include <string>
#include "../Entity.h"
#include "../Component.h"
#include "SFML/Graphics.hpp"


class BoxCollider2D : public Component
{
public:
	BoxCollider2D(sf::RenderWindow target, int width, int height)
	{
		//box.w = width;
		//box.h = height;
	}

	BoxCollider2D(sf::RenderWindow target, int width, int height, std::string tag)
	{
		collisionTag = tag;
		//box.w = width;
		//box.h = height;
	}

	~BoxCollider2D() = default;

	bool Init() override final
	{
		transform = &entity->GetComponent<Transform>();
		return true;
	}

	void draw() override final
	{
		//SDL_SetRenderDrawColor(renderTarget, 255, 214, 98, 255);
	   // SDL_RenderDrawRect(renderTarget, &box);
	}

	void update() override final
	{
		//box.x = transform->position.x;
		//box.y = transform->position.y;
	}

	std::string GetCollisionTag() const
	{
		return collisionTag;
	}


private:
	friend class Collision;
	//SDL_Rect box = { 0,0,0,0 };
	std::string collisionTag = "";
	Transform* transform = nullptr;
	// SDL_Renderer* renderTarget = nullptr;
};