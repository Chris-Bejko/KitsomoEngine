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
		entity->transform->scale = Vector2F(0.05f, 0.05f);
		rb = &entity->AddComponent<Rigidbody>(0.f);
		entity->AddComponent<BoxCollider2D>("bullet", sf::FloatRect(0, 0, 500, 500), true);
		return true;
	}

	void update(float dt) override final
	{
		timer += dt;
		if (entity->transform->position.x < 0 || entity->transform->position.x > Engine::get().GetWindow().getSize().x
			|| entity->transform->position.y < 0 || entity->transform->position.y > Engine::get().GetWindow().getSize().y)
		{
			Engine::get().GetManager()->eraseEntity(entity);
		}
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
		rb->AddForce(force * this->force);
	}

	void SetColor(sf::Color color)
	{
		lastColor = color;
		sprite->SetColor(color);
	}
	void OnCollisionEnter(BoxCollider2D& other) override final
	{
		if (other.GetCollisionTag() == "floor")
		{

			auto lastColor = other.entity->GetComponent<FloorSquare>().GetColor();
			if (lastColor == this->lastColor)
				return;
			this->lastColor = lastColor;
			sprite->SetColor(lastColor);
		}
	}

private:
	float force = 0.4f;
	sf::Color lastColor;
	float timer = 0;
	Sprite* sprite;
	Rigidbody* rb;
};