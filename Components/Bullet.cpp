#include "Bullet.h"
#include "Sprite.h"
#include "FloorSquare.h"
#include "../Timedelta.h"
#include "../Logger.h"

bool Bullet::Init()
{
	Serialize();
	entity->transform->scale = Vector2F(0.05f, 0.05f);
	// if (!entity->HasComponent<Sprite>())
	// {
	// 	sprite = &entity->AddComponent<Sprite>("circle");
	// }
	// if (!entity->HasComponent<Rigidbody>())
	// {
	// 	entity->AddComponent<Rigidbody>(0.f);
	// }
	rb = &entity->GetComponent<Rigidbody>();
	// if(!entity->HasComponent<BoxCollider>())
	// {
	// 	entity->AddComponent<BoxCollider>("bullet", sf::FloatRect(0, 0, 500, 500), true);
	// }
	return true;
}

std::vector<SerializableVariable> *Bullet::GetSerializedFields()
{
	return &variables;
}

void Bullet::InitSerializedFields(ReadableSerializableVariableMap map)
{
	for (auto const &[key, value] : map.stringFields)
	{
		if (key == "lastColorString")
		{
			lastColor.SetColor(value);
			lastColorString = value;
		}
		if (key == "force")
		{
			force = std::stof(value);
		}
	}
}
void Bullet::Serialize()
{
	variables.push_back({"force", &force, float_Type});
	variables.push_back({"lastColorString", &lastColorString, char_Type});
}
void Bullet::update(float dt)
{
	timer += dt;
	// LOG_DEBUG("Bullet position: ", entity->transform->position.x, ", ", entity->transform->position.y);

	// LOG_DEBUG("View: left=", Engine::get().GetView().left,
	//       " top=", Engine::get().GetView().top,
	//       " width=", Engine::get().GetView().width,
	//       " height=", Engine::get().GetView().height);
	if (entity->transform->position.x < Engine::get().GetView().left)
	{
	}
	if (timer < 0.5f)
		return;
	// if (entity->transform->position.x < Engine::get().GetView().left || entity->transform->position.x > Engine::get().GetView().width
	// 	|| entity->transform->position.y < Engine::get().GetView().top || entity->transform->position.y > Engine::get().GetView().height)
	// {
	// 	LOG_DEBUG("Bullet destroyed out of bounds!");
	// 	Engine::get().GetManager()->eraseEntity(entity);
	// }
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
	// LOG_INFO("Bullet color set to: ", color.SerializeColor());
}

void Bullet::OnTriggerEnter(BoxCollider &other)
{
	if (other.GetCollisionTag() == "floor")
	{
		auto lastColor = other.entity->GetComponent<FloorSquare>().GetColorEnum();
		if (lastColor == this->lastColor)
			return;
		SetColor(lastColor);
	}
}
