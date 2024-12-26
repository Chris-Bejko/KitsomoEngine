#pragma once

#include "../Camera.h"
#include "../Color.h"
#include "../Component.h"

class Player : public Component
{
public:
	Player();

	virtual ~Player() = default;

	Player(bool useControls, Vector2F position, std::string tag);

	bool Init() override final;

	void Serialize();

	std::vector<SerializableVariable>* GetSerializedFields() override final;


	void InitSerializedFields(ReadableSerializableVariableMap map);

	void Awake() override final;

	void updateEngine(float dt) override final;

	void update(float dt) override final;

	void SetSpawnPointPosition();

	void OnCollisionEnter(BoxCollider& other) override final;
	
	void OnCollisionExit(BoxCollider& other) override final;


	Vector2F GetMouseVector();

private:
	Camera* camera;
	std::vector<SerializableVariable> variables;

	bool useControls;
	Vector2F initPos;
	std::string initTag;
	Transform* spawnPoint;
	Color lastColor;
	std::string lastColorString = "";
	float cooldown = 0.5f;
	float timer;
	float moveSpeed = 350;
	sf::Vector2f mousePos;

	void Move(const Vector2F movement);
	void LookAtMouse();

};