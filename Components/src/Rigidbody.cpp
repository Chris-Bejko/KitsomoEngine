#include "Rigidbody.h"
#include "Transform.h"
#include "Logger.h"
#include "ComponentRegistry.h"

DECLARE_COMPONENT_RULES(Rigidbody, false)
REGISTER_SERIALIZABLE_COMPONENT(Rigidbody)

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
    Field("mass", mass);
    Field("gravityScale", gravityScale);
    Field("bounciness", bounciness);
    Field("friction", friction);
    Field("isStatic", isStatic);
    Field("isKinematic", isKinematic);
    Field("useGravity", useGravity);
    return true;
}

void Rigidbody::update(float dt)
{
    if (isStatic)
        return; // static objects never move

    if (initialForce.x != 0 || initialForce.y != 0)
    {
        velocity.x += initialForce.x;
        velocity.y += initialForce.y;
        initialForce = Vector2F(0, 0);
    }

    // Apply forces
    velocity.x += force.x;
    velocity.y += force.y;
    force = Vector2F(0, 0);

    // Gravity
    if (useGravity && !isKinematic)
        velocity.y += gravityScale * GRAVITY * mass * dt;

    // Drag
    if (!isKinematic)
    {
        velocity.x -= drag.x * velocity.x * dt;
        velocity.y -= drag.y * velocity.y * dt;
    }

    // Translate
    if (!isStatic)
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