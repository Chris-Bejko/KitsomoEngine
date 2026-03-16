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
	Serialize();
	return true;
}
void Rigidbody::Serialize()
{
    serializables.clear();
    serializables.push_back({ "mass",        &mass,        float_Type });
    serializables.push_back({ "gravityScale",&gravityScale,float_Type });
    serializables.push_back({ "bounciness",  &bounciness,  float_Type });
    serializables.push_back({ "friction",    &friction,    float_Type });
    serializables.push_back({ "isStatic",    &isStatic,    bool_Type  });
    serializables.push_back({ "isKinematic", &isKinematic, bool_Type  });
    serializables.push_back({ "useGravity",  &useGravity,  bool_Type  });
}

std::vector<SerializableVariable> *Rigidbody::GetSerializedFields()
{
	return &serializables;
}

void Rigidbody::InitSerializedFields(ReadableSerializableVariableMap map)
{
    for (auto const& [key, value] : map.floatFields)
    {
        if (key == "mass")         mass = value;
        if (key == "gravityScale") gravityScale = value;
        if (key == "bounciness")   bounciness = value;
        if (key == "friction")     friction = value;
    }
    for (auto const& [key, value] : map.boolFields)
    {
        if (key == "isStatic")    isStatic = value;
        if (key == "isKinematic") isKinematic = value;
        if (key == "useGravity")  useGravity = value;
    }
}

void Rigidbody::update(float dt)
{
    if (isStatic) return; // static objects never move

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