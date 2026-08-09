#pragma once

#include "Camera.h"
#include "Color.h"
#include "SerializableScript.h"
#include "Sprite.h"

class Player : public SerializableScript
{
public:
	Player();

	virtual ~Player() = default;

	Player(bool useControls, Vector2F position, std::string tag);

	bool Init() override final;

	void Awake() override final;

	void updateEngine(float dt) override final;

	void update(float dt) override final;

	void SetSpawnPointPosition();

	void OnTriggerEnter(Collider& other) override final;

	void OnTriggerExit(Collider& other) override final;

	Vector2F GetMouseVector();

private:
	Camera* camera;
	bool useControls;
	Vector2F initPos;
	std::string initTag;
	Transform* spawnPoint;
	Color lastColor;
	std::string lastColorString = "";
	float cooldown = 0.5f;
	float timer = 0.f;
	float moveSpeed = 350;

	sf::Vector2f mousePos;

	std::string bulletPrefab = "Bullet";
	std::string shootSound = "Audio/explosion.wav";
	std::string floorTouchSound = "Audio/powerUp.wav";
	Sprite* sprite;
	void Move(const Vector2F movement);
	void LookAtMouse();
	Vector2F currentVelocity = Vector2F(0, 0);

};