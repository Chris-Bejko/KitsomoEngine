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
		AssetManager::get().loadTexture("test", "test.png");
		entity->AddComponent<Sprite>("test");
		return true;
	}


	void update() override final
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			Move(Vector2F(0.1, 0));
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			Move(Vector2F(-0.1, 0));
		}

		if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			Move(Vector2F(0, -0.1));
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			Move(Vector2F(0, 0.1));
		}
	}


private:
	void Move(const Vector2F movement)
	{
		entity->transform->Translate(movement);
	}


};