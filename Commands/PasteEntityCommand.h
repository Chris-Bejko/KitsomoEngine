#pragma once
#include "Command.h"
#include "EntityClipboard.h"
#include "Entity.h"
#include "Engine.h"
#include "Vector2.h"
#include "Transform.h"
#include "ComponentRegistry.h"
class PasteEntityCommand : public Command
{
public:
    PasteEntityCommand(float offsetX = 10.0f, float offsetY = 0.0f)
        : offset(offsetX, offsetY), pastedEntity(nullptr)
    {
        if (EntityClipboard::get().HasContent())
        {
            savedClipboard = EntityClipboard::get().GetClipboard();
        }
    }

    void Execute() override
    {
        if (!EntityClipboard::get().HasContent())
            return;

        auto clipboard = EntityClipboard::get().GetClipboard();

        std::string newName = Engine::get().GetManager()->GetUniqueName(clipboard.name);

        pastedEntity = new Entity(newName);

        for (const auto &serialized : clipboard.components)
        {
            auto it = ComponentRegistry::get().GetAll().find(serialized.GetType());

            if (it == ComponentRegistry::get().GetAll().end())
            {
                LOG_WARNING("PasteEntity: Unknown component: ", serialized.GetType().c_str());

                continue;
            }

            it->second.factory(pastedEntity, serialized);
        }

        // Override position with mouse world position
        auto mousePixel = sf::Mouse::getPosition(Engine::get().GetWindow());

        auto mouseWorld = Engine::get().GetWindow().mapPixelToCoords(mousePixel);

        if (pastedEntity->HasComponent<Transform>())
        {
            pastedEntity->GetComponent<Transform>().position = Vector2F(mouseWorld.x, mouseWorld.y);
        }

        Engine::get().Spawn(pastedEntity);
        Engine::get().GetManager()->SetSelectedEntity(pastedEntity);
    }

    void Undo() override
    {
        if (pastedEntity)
        {
            pastedEntity->Destroy();
            pastedEntity = nullptr;
        }
    }

    std::string GetDescription() const override
    {
        return "Paste Entity: " + savedClipboard.name;
    }

private:
    Entity *pastedEntity;
    Vector2F offset;
    SerializedEntityClipboard savedClipboard;
};
