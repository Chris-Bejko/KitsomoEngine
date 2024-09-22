#pragma once

#include "Player.h"
class FloorSquare : public Component
{
public:
	FloorSquare() = default;

	virtual ~FloorSquare() = default;
	bool Init() override final
	{
		AssetManager::get().loadTexture("square", "square.png");
		sprite = &entity->AddComponent<Sprite>("square");
		entity->transform->scale = Vector2F(0.2, 0.2);


		return true;
	}

	void Config(Vector2F position, sf::Color color)
	{
		this->color = color;
		std::cout << color.a << std::endl;
		entity->transform->position = position;
		entity->AddComponent<BoxCollider>("floor", sf::FloatRect(0, 0, 125, 125), true);
		sprite->SetColor(color);
	}


	sf::Color GetColor()
	{
		return color;
	}

	void OnTriggerStay(BoxCollider& other) override final
	{
		std::cout << "On Trigger Stay" << std::endl;
	}
private:
	sf::Color color;
	Sprite* sprite;
};