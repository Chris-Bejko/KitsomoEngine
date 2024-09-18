#pragma once

#include "ECSEngine.h"
#include "Player.h"
class FloorSquare : public Component
{
public:

	bool Init() override final
	{
		AssetManager::get().loadTexture("square", "square.png");
		sprite = &entity->AddComponent<Sprite>("square");
		entity->transform->scale = Vector2F(0.2, 0.2);
		entity->AddComponent<BoxCollider2D>("floor", sf::FloatRect(0, 0, 125, 125), true);

		return true;
	}

	void Config(Vector2F position, sf::Color color)
	{
		this->color = color;
		std::cout << color.a << std::endl;
		entity->transform->position = position;
		sprite->SetColor(color);
	}


	sf::Color GetColor()
	{
		//std::cout << color.g << std::endl;

		return color;
	}

	void OnTriggerStay(BoxCollider2D& other) override final
	{
		std::cout << "On Trigger Stay" << std::endl;
	}
private:
	sf::Color color;
	Sprite* sprite;
};