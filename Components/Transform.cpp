#include "Transform.h"
#include "Logger.h"
#include "Entity.h"

Transform::Transform(float x, float y)
{
	position.x = x;
	position.y = y;
}

Transform::Transform(float x, float y, float scX, float scY)
{
	position.x = x;
	position.y = y;
	scale.x = scX;
	scale.y = scY;
	rotation = 0;
}

Transform::Transform(float x, float y, float scX, float scY, float roation)
{
	position.x = x;
	position.y = y;
	scale.x = scX;
	scale.y = scY;
	this->rotation = rotation;
}

void Transform::SerializeVariables()
{
	variables.push_back({ "position.x", &position.x, float_Type });
	variables.push_back({ "position.y", &position.y, float_Type });
	variables.push_back({ "rotation", &rotation, float_Type });
	variables.push_back({ "scale.x", &scale.x, float_Type });
	variables.push_back({ "scale.y", &scale.y, float_Type });
}

void Transform::InitSerializedFields(ReadableSerializableVariableMap variables)
{
	for (auto const& [key, value] : variables.floatFields)
	{
		if (key == "position.x")
		{
			position.x = value;
			//this->variables.push_back({ f.name , &position.x, float_Type });
		}
		if (key == "position.y")
		{
			position.y = value;
			//this->variables.push_back({ f.name , &position.y, float_Type });
		}
		if (key == "rotation")
		{
			rotation = value;
			//this->variables.push_back({ f.name , &rotation, float_Type });
		}
		if (key == "scale.x")
		{
			scale.x = value;
			//this->variables.push_back({ f.name , &scale.x, float_Type });
		}
		if (key == "scale.y")
		{
			scale.y = value;
			//this->variables.push_back({ f.name , &scale.y, float_Type });
		}
	}
}

std::vector<SerializableVariable>* Transform::GetSerializedFields()
{
	return &variables;
}

bool Transform::Init()
{
	//tag = "Test Tag";
	SerializeVariables();
	return true;
}

void Transform::Translate(const Vector2F velocity)
{
	position += velocity;
}

void Transform::LookAt(const float rotation)
{
	this->rotation = rotation;
}

void Transform::SetPosition(float x, float y)
{
	position.x = x;
	position.y = y;
}

void Transform::SetParent(Transform* newParent)
{
    // Remove from old parent
    if (parent)
        parent->RemoveChild(this);

    parent = newParent;

    if (parent)
        parent->AddChild(this);
}

void Transform::AddChild(Transform* child)
{
    // Avoid duplicates
    for (auto* c : children)
        if (c == child) return;
    children.push_back(child);
}

void Transform::RemoveChild(Transform* child)
{
    children.erase(
        std::remove(children.begin(), children.end(), child),
        children.end()
    );
}

void Transform::ClearHierarchy()
{
    parent = nullptr;
    children.clear();
}

Vector2F Transform::GetWorldPosition()
{
    if (parent == nullptr)
        return position;

    Vector2F parentWorld = parent->GetWorldPosition();
    float parentRot = parent->GetWorldRotation() * 3.14159f / 180.f;
    Vector2F parentScale = parent->GetWorldScale();

    // Rotate and scale local position by parent's world transform
    float rotatedX = position.x * parentScale.x * cos(parentRot) 
                   - position.y * parentScale.y * sin(parentRot);
    float rotatedY = position.x * parentScale.x * sin(parentRot) 
                   + position.y * parentScale.y * cos(parentRot);

    return Vector2F(parentWorld.x + rotatedX, parentWorld.y + rotatedY);
}

float Transform::GetWorldRotation()
{
    if (parent == nullptr)
        return rotation;
    return rotation + parent->GetWorldRotation();
}

Vector2F Transform::GetWorldScale()
{
    if (parent == nullptr)
        return scale;
    Vector2F parentScale = parent->GetWorldScale();
    return Vector2F(scale.x * parentScale.x, scale.y * parentScale.y);
}