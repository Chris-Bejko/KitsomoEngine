#pragma once
#include "Command.h"
#include "../Entity.h"
#include "../Engine.h"
#include "../Vector2.h"
#include "../Components/Transform.h"
class DuplicateEntityCommand : public Command
{
public:
    DuplicateEntityCommand(Entity *original, float offsetX = 10.0f, float offsetY = 0.0f)
        : originalEntity(original), offset(offsetX, offsetY), duplicatedEntity(nullptr)
    {
    }

    void Execute() override
    {
        if (!originalEntity)
            return;

        std::string newName = Engine::get().GetManager()->GetUniqueName(originalEntity->GetName());
        auto components = originalEntity->GetAllComponentVariables();

        duplicatedEntity = new Entity(newName);

        for (const auto &comp : components)
        {
            if (comp.componentName == "Transform")
                continue;
            duplicatedEntity->AddComponentByName(comp.componentName);
        }

        duplicatedEntity->ValidateAddedComponents();
        duplicatedEntity->InitializeComponentFields(components);

        // Place at mouse position
        auto mousePixel = sf::Mouse::getPosition(Engine::get().GetWindow());
        auto mouseWorld = Engine::get().GetWindow().mapPixelToCoords(mousePixel);
        if (duplicatedEntity->HasComponent<Transform>())
            duplicatedEntity->GetComponent<Transform>().position = Vector2F(mouseWorld.x, mouseWorld.y);

        Engine::get().GetManager()->ClearInspector();

        Engine::get().Spawn(duplicatedEntity);
        Engine::get().GetManager()->SetSelectedEntity(duplicatedEntity);
    }

    void Undo() override
    {
        if (duplicatedEntity)
        {
            duplicatedEntity->Destroy();
            duplicatedEntity = nullptr;
            Engine::get().GetManager()->SetSelectedEntity(originalEntity);
        }
    }

    std::string GetDescription() const override
    {
        return "Duplicate Entity: " + originalEntity->GetName();
    }

private:
    Entity *originalEntity;
    Entity *duplicatedEntity;
    Vector2F offset;
};
