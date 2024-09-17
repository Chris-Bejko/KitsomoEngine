#pragma once

#include "ECSEngine.h"
class FloorSquare : public Component
{
public:

	bool Init() override final
	{
		AssetManager::get().loadTexture("square", "square.png");
		entity->AddComponent<Sprite>();

	}

	void Config(Vector2F position, sf::Color color)
	{
		this->color = color;
		entity->transform->position = position;
	}


private:
	sf::Color color;
};