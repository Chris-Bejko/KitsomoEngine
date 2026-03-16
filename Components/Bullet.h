#pragma once
#include "../Engine.h"
#include "../Color.h"
#include "../Vector2.h"
#include "Sprite.h"
#include "Rigidbody.h"

class Bullet : public Component
{

public:
	Bullet() = default;
	virtual ~Bullet() = default;


	bool Init() override final;

	void update(float dt) override final;

	void SetRotation(float rotation);

	void SetPosition(Vector2F position);

	void AddForce(Vector2F force);

	void SetColor(Color color);

	void OnTriggerEnter(Collider& other) override final;
	void OnCollisionEnter(Collider& other) override final;

	float GetForce() { return force; }	

	void InitSerializedFields(ReadableSerializableVariableMap map) override final;
	std::vector<SerializableVariable>* GetSerializedFields() override final;

	void Serialize();
private:
	float force = 850.f;
	Color lastColor;
	std::string lastColorString;
	float timer = 0;
	Sprite* sprite;
	Rigidbody* rb;
	std::vector<SerializableVariable> variables;

};