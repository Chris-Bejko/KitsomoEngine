#pragma once
#include "Component.h"
#include "Components/Transform.h"

class Enemy : Component
{
public:
	Enemy() = default;

	virtual ~Enemy() = default;

	void update(float dt) override final;

	void Awake() override final;

	bool Init() override final;
	
	void SetFollowTarget(Transform* target);

private:
	Transform* target;	

};