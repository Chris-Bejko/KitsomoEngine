#pragma once

#include "ECSEngine.h"
#include "FloorSquare.h"
#include "Bullet.h"
#include <math.h>

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
		entity->AddComponent<BoxCollider>(initTag, sf::FloatRect(0, 0, 55, 50));
		entity->transform->scale = Vector2F(0.05, 0.05);
		Entity* bulletSpawnPoint = new Entity("spawnpoint");
		AssetManager::get().loadTexture("circle", "circle.png");
		bulletSpawnPoint->AddComponent<Sprite>("circle");
		spawnPoint = bulletSpawnPoint->transform;
		spawnPoint->scale = Vector2F(.03f, .03f);
		Engine::get().Spawn(bulletSpawnPoint);

		return true;
	}


	void update(float dt) override final
	{
		if (!useControls)
			return;

		SetSpawnPointPosition();
		timer += dt;
		std::cout << sf::Mouse::isButtonPressed(sf::Mouse::Left) << " , " << (timer >= cooldown) << std::endl;
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && timer >= cooldown)
		{
			timer = 0;

			Entity* bullet = new Entity("Bullet");
			std::cout << "Space pressed" << std::endl;

			auto spawned = &bullet->AddComponent<Bullet>();
			spawned->SetPosition(spawnPoint->position);
			spawned->SetRotation(entity->transform->rotation);
			spawned->SetColor(lastColor);
			Engine::get().GetManager()->addEntity(bullet);
			spawned->AddForce(GetMouseVector());
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

	void SetSpawnPointPosition()
	{
		auto bounds = entity->GetComponent<Sprite>().GetGlobalBounds();
		spawnPoint->entity->GetComponent<Sprite>().SetOrigin(Vector2F(bounds.left, bounds.top));
		sf::Transform rot;
		rot.rotate(entity->transform->rotation, entity->transform->position.x, entity->transform->position.y);
		sf::Vector2f point = rot.transformPoint(bounds.left + bounds.width / 2, bounds.height / 2);

		spawnPoint->position = Vector2F(point.x, point.y);
		spawnPoint->rotation = entity->transform->rotation;
	}

	void OnCollisionEnter(BoxCollider& other) override final
	{
		if (other.entity->HasComponent<FloorSquare>())
		{
			lastColor = other.entity->GetComponent<FloorSquare>().GetColor();
			entity->GetComponent<Sprite>().SetColor(lastColor);
		}
	}

	void OnCollisionExit(BoxCollider& other) override final
	{
		entity->GetComponent<Sprite>().SetColor(sf::Color::Red);
	}

	Vector2F GetMouseVector()
	{
		auto mousePos = Vector2F(this->mousePos.x, this->mousePos.y);
		auto origin = Vector2F(entity->GetComponent<Sprite>().GetOrigin().x, entity->GetComponent<Sprite>().GetOrigin().y);
		Vector2F aimDir = mousePos - origin;
		auto aimDirNorm = aimDir / sqrt(pow(aimDir.x, 2) + pow(aimDir.y, 2));
		return aimDirNorm;
	}

private:
	bool useControls;
	Vector2F initPos;
	std::string initTag;
	Transform* spawnPoint;
	sf::Color lastColor;
	float cooldown = 0.2f;
	float timer;
	float moveSpeed = 1000;
	sf::Vector2i mousePos;

	void Move(const Vector2F movement)
	{
		entity->transform->Translate(movement);
	}

	void LookAtMouse()
	{
		mousePos = sf::Mouse::getPosition(Engine::get().GetWindow());
		auto mouseAngle = -atan2(mousePos.x - entity->transform->position.x, mousePos.y - entity->transform->position.y) * 180 / 3.14159;

		entity->transform->LookAt(mouseAngle + 180);
	}

};