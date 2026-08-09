#pragma once
#include "Command.h"
#include "../Base/include/Entity.h"
#include "../Components/Transform.h"
#include "../DataClasses/include/Vector2.h"

class MoveEntityCommand : public Command
{
public:
	MoveEntityCommand(Entity* entity, const Vector2F& fromPos, const Vector2F& toPos)
		: targetEntity(entity), startPosition(fromPos), endPosition(toPos)
	{
	}

	void Execute() override
	{
		if (targetEntity && targetEntity->HasComponent<Transform>())
		{
			auto& transform = targetEntity->GetComponent<Transform>();
			transform.position = endPosition;
		}
	}

	void Undo() override
	{
		if (targetEntity && targetEntity->HasComponent<Transform>())
		{
			auto& transform = targetEntity->GetComponent<Transform>();
			transform.position = startPosition;
		}
	}

	std::string GetDescription() const override
	{
		return "Move Entity";
	}

private:
	Entity* targetEntity;
	Vector2F startPosition;
	Vector2F endPosition;
};
