#pragma once

#include "ECSEngine.h"

class Player : public Component
{
public:
	Player() = default;
	virtual ~Player() = default;



	bool Init() override final
	{
		entity->transform->position = Vector2F(100, 100);
		AssetManager::get().loadTexture("test", "C:\\Users\\kitso\\Documents\\CPP Game\\SDL ECS\\Assets\\test.png");
		entity->AddComponent<Sprite>("test");
		return true;
	}


	void update() override final
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			entity->transform->Translate(Vector2F(0.1, 0));
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			entity->transform->Translate(Vector2F(-0.1, 0));
		}

		if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			entity->transform->Translate(Vector2F(0, -0.1));
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			entity->transform->Translate(Vector2F(0, 0.1));
		}
	}



};