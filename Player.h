#pragma once

#include "ECSEngine.h"
#include "FloorSquare.h"

class Player : public Component
{
public:
	Player() = default;
	virtual ~Player() = default;

	Player(bool useControls, Vector2F position, std::string tag)
	{
		this->useControls = useControls;
		initPos = position;
		initTag = tag;
	}

	bool Init() override final
	{
		entity->transform->position = initPos;
		AssetManager::get().loadTexture("triangle", "triangle.png");
		entity->AddComponent<Sprite>("triangle");
		entity->AddComponent<BoxCollider2D>(initTag, sf::FloatRect(0, 0, 55, 50));
		entity->transform->scale = Vector2F(0.05, 0.05);
		return true;
	}


	void update() override final
	{
		if (!useControls)
			return;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			Move(Vector2F(0.1, 0));
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			Move(Vector2F(-0.1, 0));
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			Move(Vector2F(0, -0.1));
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			Move(Vector2F(0, 0.1));
		}

		LookAtMouse();
	}

	void OnCollisionEnter(BoxCollider2D& other) override final
	{
		if(other.entity->HasComponent<FloorSquare>())
		{
			entity->GetComponent<Sprite>().SetColor(other.entity->GetComponent<FloorSquare>().GetColor());
			//std::cout << other.entity->GetComponent<FloorSquare>().GetColor().toInteger() << std::endl;
		}
	}

	void OnCollisionExit(BoxCollider2D& other) override final
	{
		entity->GetComponent<Sprite>().SetColor(sf::Color::Red);
	}

private:
	bool useControls;
	Vector2F initPos;
	std::string initTag;
	void Move(const Vector2F movement)
	{
		entity->transform->Translate(movement);
	}

	void LookAtMouse()
	{
		auto mousePos = sf::Mouse::getPosition(Engine::get().GetWindow());
		auto mouseAngle = -atan2(mousePos.x - entity->transform->position.x, mousePos.y - entity->transform->position.y) * 180 / 3.14159;

		entity->transform->LookAt(mouseAngle + 180);
	}

};