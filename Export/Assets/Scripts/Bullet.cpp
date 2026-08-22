#include "Bullet.h"
#include "Sprite.h"
#include "FloorSquare.h"
#include "Timedelta.h"
#include "Logger.h"
#include "Engine.h"

bool Bullet::Init()
{
	Field("lastColorString", lastColorString);
	Field("force", force);
	entity->transform->scale = Vector2F(0.05f, 0.05f);
	rb = &entity->GetComponent<Rigidbody>();
	return true;
}

void Bullet::update(float dt)
{
	timer += dt;
	if (entity->transform->position.x < Engine::get().GetView().left)
	{
	}
	if (timer < 0.5f)
		return;
		
	if (timer > 5)
		Engine::get().GetManager()->eraseEntity(entity);
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
    if (!entity->HasComponent<Rigidbody>())
    {
        LOG_ERROR("Bullet has no Rigidbody!");
        return;
    }
    rb = &entity->GetComponent<Rigidbody>();
    rb->AddInitialForce(force * this->force);
}

void Bullet::SetColor(Color color)
{
	lastColor = color;
	lastColorString = color.SerializeColor();
	sprite = &entity->GetComponent<Sprite>();
	sprite->SetColor(color);
}

void Bullet::OnTriggerEnter(Collider &other)
{
	LOG_DEBUG("Bullet OnTriggerEnter called! other tag: ", other.GetCollisionTag().c_str());

	if (other.GetCollisionTag() == "floor")
	{
		LOG_INFO("Bullet hit the floor, changing color to match the floor's color");
		auto lastColor = other.entity->GetComponent<FloorSquare>().GetColorEnum();
		if (lastColor == this->lastColor)
			return;
		SetColor(lastColor);
	}
}
void Bullet::OnCollisionEnter(Collider &other)
{
	LOG_DEBUG("Bullet OnCollisionEnter called! other tag: ", other.GetCollisionTag().c_str());

	if (other.GetCollisionTag() == "floor")
	{
		LOG_INFO("Bullet hit the floor, changing color to match the floor's color");
		auto lastColor = other.entity->GetComponent<FloorSquare>().GetColorEnum();
		if (lastColor == this->lastColor)
			return;
		SetColor(lastColor);
	}
}
