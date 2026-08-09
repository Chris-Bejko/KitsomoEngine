#pragma once
#include "Command.h"
#include "Entity.h"
#include "Transform.h"
#include "Vector2.h"

class ScaleEntityCommand : public Command
{
public:
	ScaleEntityCommand(Entity* entity, const Vector2F& fromScale, const Vector2F& toScale)
		: targetEntity(entity), startScale(fromScale), endScale(toScale)
	{
	}

	void Execute() override
	{
		if (targetEntity && targetEntity->HasComponent<Transform>())
		{
			auto& transform = targetEntity->GetComponent<Transform>();
			transform.scale = endScale;
		}
	}

	void Undo() override
	{
		if (targetEntity && targetEntity->HasComponent<Transform>())
		{
			auto& transform = targetEntity->GetComponent<Transform>();
			transform.scale = startScale;
		}
	}

	std::string GetDescription() const override
	{
		return "Rotate Entity";
	}

private:
	Entity* targetEntity;
	Vector2F startScale;
	Vector2F endScale;
};
