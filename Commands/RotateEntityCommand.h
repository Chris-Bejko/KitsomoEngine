#pragma once
#include "Command.h"
#include "Entity.h"
#include "Transform.h"
#include "Vector2.h"

class RotateEntityCommand : public Command
{
public:
	RotateEntityCommand(Entity* entity, float fromRotation, float toRotation)
		: targetEntity(entity), startRotation(fromRotation), endRotation(toRotation)
	{
	}

	void Execute() override
	{
		if (targetEntity && targetEntity->HasComponent<Transform>())
		{
			auto& transform = targetEntity->GetComponent<Transform>();
			transform.rotation = endRotation;
		}
	}

	void Undo() override
	{
		if (targetEntity && targetEntity->HasComponent<Transform>())
		{
			auto& transform = targetEntity->GetComponent<Transform>();
			transform.rotation = startRotation	;
		}
	}

	std::string GetDescription() const override
	{
		return "Rotate Entity";
	}

private:
	Entity* targetEntity;
	float startRotation;
	float endRotation;
};
