#pragma once
#include "ECSEngine.h"


class Bullet : public Component
{

public:
	Bullet() = default;
	~Bullet() = default;


	bool Init() override final
	{
		sprite = &entity->AddComponent<Sprite>("circle");
		entity->transform->scale = Vector2F(0.1, 0.1f);
		rb = &entity->AddComponent<Rigidbody>(0.f);
//		entity->AddComponent<BoxCollider2D>("bullet", sf::FloatRect(0, 0, 44, 44), true);
		return true;
	}

	void update(float dt) override final
	{
		timer += dt;
		//if(timer > 5)
			//Engine::get().GetManager()->eraseEntity(entity);

	}
	void SetRotation(float rotation)
	{
		entity->transform->rotation = rotation;
	}
	void SetPosition(Vector2F position)
	{
		entity->transform->position = position;
	}
	void AddForce(Vector2F force)
	{
		rb->AddForce(force);
	}

	void SetColor(sf::Color color)
	{
		sprite->SetColor(color);
	}
	void OnTriggerEnter(BoxCollider2D& other)
	{
	}

private:
	float timer = 0;
	Sprite* sprite;
	Rigidbody* rb;
};