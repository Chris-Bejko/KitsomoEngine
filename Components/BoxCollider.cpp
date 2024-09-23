#include "BoxCollider.h"
#include "../Engine.h"
#include "Sprite.h"

BoxCollider::BoxCollider(std::string tag, bool isTrigger)
{
	collisionTag = tag;
	configuredHitbox = false;
	this->isTrigger = isTrigger;
}

BoxCollider::BoxCollider(const std::string tag, sf::FloatRect hitbox, bool isTrigger)
{
	collisionTag = tag;
	this->hitbox = hitbox;
	configuredHitbox = true;
}

bool BoxCollider::Init()
{
	if (entity->HasComponent<Sprite>())
	{
		if (!configuredHitbox)
		{
			hitbox = entity->GetComponent<Sprite>().GetGlobalBounds();
		}

	}

	SetUpColliderVisuals();
	//transform = &entity->GetComponent<Transform>();
	Serialize();

	return true;
}

void BoxCollider::Serialize()
{
	serializables.clear();
	serializables.push_back({ "hitbox.top", &hitbox.top, float_Type });
	serializables.push_back({ "hitbox.left", &hitbox.left, float_Type });
	serializables.push_back({ "hitbox.width", &hitbox.width, float_Type });
	serializables.push_back({ "hitbox.height", &hitbox.height, float_Type });
	serializables.push_back({ "collisionTag", &collisionTag, char_Type });
	serializables.push_back({ "isTrigger", &isTrigger, bool_Type });
}

std::vector<SerializableVariable>* BoxCollider::GetSerializedFields()
{
	return &serializables;
}

void BoxCollider::InitSerializedFields(ReadableSerializableVariableMap map)
{
	for (auto const& [key, value] : map.floatFields)
	{
		if (key == "hitbox.top")
		{
			hitbox.top = value;
		}
		if (key == "hitbox.left")
		{
			hitbox.left = value;
		}
		if (key == "hitbox.width")
		{
			hitbox.width = value;
		}
		if (key == "hitbox.height")
		{
			hitbox.height = value;
		}
	}
	for (auto const& [key, value] : map.stringFields)
	{
		if (key == "collisionTag")
		{
			collisionTag = value;
		}
	}

	for (auto const& [key, value] : map.boolFields)
	{
		if (key == "isTrigger")
		{
			isTrigger = value;
		}
	}
}

void BoxCollider::SetUpColliderVisuals()
{
	colliderVisual.setFillColor(sf::Color::Transparent);
	colliderVisual.setOutlineColor(sf::Color::Green);
	colliderVisual.setOutlineThickness(1);
	//colliderVisual.setPosition(sf::Vector2f(transform->position.x, transform->position.y));
	colliderVisual.setSize(sf::Vector2f(hitbox.width * entity->transform->scale.x, hitbox.height * entity->transform->scale.y));
	//colliderVisual.setOrigin(sprite->GetOrigin());
}

void BoxCollider::draw()
{
	if (!Engine::get().isEngine)
		return;

	colliderVisual.setRotation(entity->transform->rotation);
	Engine::get().GetWindow().draw(colliderVisual);
}

void BoxCollider::update(float dt)
{
	//colliderVisual.setPosition(sprite->GetPosition());
	//colliderVisual.setOrigin(sprite->GetOrigin());
	colliderVisual.setPosition(GetRect().getPosition());
}

std::string BoxCollider::GetCollisionTag()
{
	return collisionTag;
}

sf::FloatRect BoxCollider::GetRect()
{
	return entity->GetComponent<Sprite>().TranslateHitbox(hitbox);
}
