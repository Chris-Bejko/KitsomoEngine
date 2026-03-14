#include "Rigidbody.h"
#include "Transform.h"
#include "Logger.h"

Rigidbody::Rigidbody()
{
	gravityScale = 1.0f;
}

Rigidbody::Rigidbody(float gravityScale)
{
	this->gravityScale = gravityScale;
}

bool Rigidbody::Init()
{
	return true;
}

void Rigidbody::update(float dt)
{
	if (initialForce.x != 0 || initialForce.y != 0)
	{
		velocity.x += initialForce.x;
		velocity.y += initialForce.y;
		initialForce = Vector2F(0, 0); // clear after applying
	}
	velocity.x += force.x;
	velocity.y += force.y + gravityScale * GRAVITY * mass * dt;
	velocity.x -= drag.x * velocity.x * dt;
	velocity.y -= drag.y * velocity.y * dt;
	force = Vector2F(0, 0);
	entity->GetComponent<Transform>().Translate(velocity * dt);
}

void Rigidbody::SetForce(const Vector2F force)
{
	this->force = force;
}

void Rigidbody::AddInitialForce(Vector2F f)
{
	initialForce = f;
}

void Rigidbody::SetForce(float x, float y)
{
	this->force.x = x;
	this->force.y = y;
}

void Rigidbody::AddForce(const Vector2F force)
{
	this->force += force;
}

Vector2F Rigidbody::GetVelocity()
{
	return velocity;
}

Vector2F Rigidbody::GetForce()
{
	return this->force;
}