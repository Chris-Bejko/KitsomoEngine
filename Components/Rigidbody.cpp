#include "Rigidbody.h"

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
	velocity.x = force.x - drag.x * dt;
	velocity.y = force.y + drag.y + gravityScale * GRAVITY * mass * dt;
	entity->GetComponent<Transform>().Translate(velocity);
}

void Rigidbody::SetForce(const Vector2F force)
{
	this->force = force;
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
