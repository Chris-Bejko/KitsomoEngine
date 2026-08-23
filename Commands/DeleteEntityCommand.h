#pragma once
#include "Command.h"
#include "EntityClipboard.h"
#include "Entity.h"
#include "Engine.h"
#include "Vector2.h"
#include "Transform.h"
#include <regex>

class DeleteEntityCommand : public Command
{
public:
DeleteEntityCommand(Entity* entity) : entityToDelete(entity)
{
    if (entity)
    {
        savedName = entity->GetName();

        for (const auto& component : entity->GetComponents())
        {
            if (!component)
                continue;

            SerializedComponent serialized;
            std::string componentName = typeid(*component).name();
            componentName = std::regex_replace(componentName, std::regex("class "), "");
            serialized.SetType(componentName);
            serialized.SetGUID(component->GetGUID());

            for (const auto& field : component->GetSerializedFields())
            {
                if (field)
                    serialized.AddSerializedField(field->GetName(), field->Serialize());
            }

            savedComponents.push_back(std::move(serialized));
        }

        if (entity->HasComponent<Transform>())
        {
            auto& transform = entity->GetComponent<Transform>();
            savedPosition = transform.position;
            savedRotation = transform.rotation;
            savedScale = transform.scale;
        }
    }
}

	void Execute() override
	{
		if (entityToDelete)
		{
			entityToDelete->Destroy();
			Engine::get().GetManager()->SetSelectedEntity(nullptr);
		}
	}

	void Undo() override
	{
		// Recreate the entity
		Entity* newEntity = new Entity(savedName);
		
		// Restore components
		for (const auto& comp : savedComponents)
		{
			newEntity->AddComponentByName(comp.GetType());
			// The components will be re-initialized with their saved data
		}
		
		newEntity->ValidateAddedComponents();
		
		// Restore transform if it exists
		if (newEntity->HasComponent<Transform>())
		{
			auto& transform = newEntity->GetComponent<Transform>();
			transform.position = savedPosition;
			transform.rotation = savedRotation;
			transform.scale = savedScale;
		}
		
		Engine::get().Spawn(newEntity);
		entityToDelete = newEntity;
	}

	std::string GetDescription() const override
	{
		return "Delete Entity: " + savedName;
	}

private:
	Entity* entityToDelete;
	std::string savedName;
	std::vector<SerializedComponent> savedComponents;
	Vector2F savedPosition;
	float savedRotation = 0.0f;
	Vector2F savedScale;
};
