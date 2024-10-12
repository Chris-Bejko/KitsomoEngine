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

	void OnCollisionEnter(BoxCollider& other) override final;

private:
	float force = 5.f;
	Color lastColor;
	float timer = 0;
	Sprite* sprite;
	Rigidbody* rb;
};