#pragma once

#include "ECSEngine.h"
#include "FloorSquare.h"
#include "Bullet.h"

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
		Entity* bulletSpawnPoint = new Entity();
		AssetManager::get().loadTexture("circle", "circle.png");
		spawnPoint = bulletSpawnPoint->transform;
		Engine::get().Spawn(bulletSpawnPoint);
		return true;
	}


	void update(float dt) override final
	{
		if (!useControls)
			return;

		spawnPoint->position = entity->transform->position;
		spawnPoint->rotation = entity->transform->rotation;
		timer += dt;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && timer > cooldown)
		{
			timer = 0;

			Entity* bullet = new Entity();
			std::cout << "Space pressed" << std::endl;

			auto spawned = &bullet->AddComponent<Bullet>();
			spawned->SetPosition(spawnPoint->position);
			spawned->SetRotation(entity->transform->rotation);
			spawned->SetColor(lastColor);
			Engine::get().GetManager()->addEntity(bullet);
			spawned->AddForce(Vector2F(0.2, entity->transform->rotation));
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			Move(Vector2F(moveSpeed * dt, 0));
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			Move(Vector2F(-moveSpeed * dt, 0));
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			Move(Vector2F(0, -moveSpeed * dt));
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			Move(Vector2F(0, moveSpeed * dt));
		}

		LookAtMouse();
	}

	void OnCollisionEnter(BoxCollider2D& other) override final
	{
		if (other.entity->HasComponent<FloorSquare>())
		{
			lastColor = other.entity->GetComponent<FloorSquare>().GetColor();
			entity->GetComponent<Sprite>().SetColor(lastColor);
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
	Transform* spawnPoint;
	sf::Color lastColor;
	float cooldown = 1;
	float timer;
	float moveSpeed = 1000;
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