#include "Bullet.h"
#include "Sprite.h"
#include "FloorSquare.h"
#include "../Timedelta.h"
#include "../Logger.h"

bool Bullet::Init()
{
	sprite = &entity->AddComponent<Sprite>("circle");
	entity->transform->scale = Vector2F(0.05f, 0.05f);
	rb = &entity->AddComponent<Rigidbody>(0.f);
	entity->AddComponent<BoxCollider>("bullet", sf::FloatRect(0, 0, 500, 500), true);
	return true;
}

void Bullet::update(float dt)
{
	timer += dt;
	LOG_DEBUG("Bullet pos: ", entity->transform->position.x, ", ", entity->transform->position.y);
	if (entity->transform->position.x < Engine::get().GetView().left )
	{
		
	}
	timer += dt;
	if (entity->transform->position.x < Engine::get().GetView().left || entity->transform->position.x > Engine::get().GetView().width
		|| entity->transform->position.y < Engine::get().GetView().top || entity->transform->position.y > Engine::get().GetView().height)
	{
		Engine::get().GetManager()->eraseEntity(entity);
		LOG_DEBUG("Bullet destroyed out of bounds!");
		Engine::get().GetManager()->eraseEntity(entity);
	}
	//if(timer > 5)
		//Engine::get().GetManager()->eraseEntity(entity);

}

void Bullet::SetRotation(float rotation)
{
	entity->transform->rotation = rotation;
}

void Bullet::SetPosition(Vector2F position)
{
	entity->transform->position = position;
}

void Bullet::AddForce(Vector2F force)
{
	rb->AddForce(force * this->force);
}

void Bullet::SetColor(Color color)
{
	lastColor = color;
	sprite->SetColor(color);

}

void Bullet::OnCollisionEnter(BoxCollider& other)
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
