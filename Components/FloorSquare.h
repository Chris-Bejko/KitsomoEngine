#pragma once

#include "../Color.h"
#include "Sprite.h"
#include "Collision/Collider.h"
#include "../SerializableScript.h"

class FloorSquare : public SerializableScript
{
public:
	FloorSquare() = default;

	virtual ~FloorSquare() = default;
	bool Init() override final;

	void Config(Vector2F position, Color color);

	void update(float dt) override final;

	void updateEngine(float dt) override final;


	Color GetColorEnum();

	void OnTriggerStay(Collider &other) override final;
	void OnTriggerEnter(Collider &other) override final;

private:
	Color color;
	std::string colorString;

	Sprite *sprite;
};