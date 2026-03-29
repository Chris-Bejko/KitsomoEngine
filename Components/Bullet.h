#pragma once
#include "../Color.h"
#include "../Vector2.h"
#include "Sprite.h"
#include "Rigidbody.h"
#include "../SerializableScript.h"

class Bullet : public SerializableScript
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

	std::string GetColorString() {return lastColorString; }
private:
	float force = 850.f;
	Color lastColor;
	std::string lastColorString;
	float timer = 0;
	Sprite* sprite;
	Rigidbody* rb;

};