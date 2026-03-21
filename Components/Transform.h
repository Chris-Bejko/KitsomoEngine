#pragma once
#include "../Vector2.h"
#include "../SerializableScript.h"

class Transform : public SerializableScript
{
public:
	Transform() = default;
	Transform(float x, float y);
	Transform(float x, float y, float scX, float scY);

	Transform(float x, float y, float scX, float scY, float roation);
	virtual ~Transform() = default;

	bool Init() override final;

	Vector2F position = Vector2F();
	Vector2F scale = Vector2F(1.0f, 1.0f);
	float rotation = 0.0f;

	void Translate(const Vector2F velocity);

	void LookAt(const float rotation);

	void SetPosition(float x, float y);


	void SetParent(Transform* newParent);

	void AddChild(Transform* child);	
	void RemoveChild(Transform* child);

	Vector2F GetWorldPosition();
	Vector2F GetWorldScale();
	float GetWorldRotation();
	void ClearHierarchy();
	Transform* GetParent() { return parent; }
	std::vector<Transform*>& GetChildren() { return children; }
	bool isUITransform = false;
private:
	Transform* parent = nullptr;
	std::vector<Transform*> children;
	


};