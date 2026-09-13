#pragma once
#include "Command.h"
#include "EntityClipboard.h"
#include "Entity.h"
#include <regex>
class CopyEntityCommand : public Command
{
public:
    CopyEntityCommand(Entity *entity) : entityToCopy(entity)
    {
    }

    void Execute() override
    {
        if (!entityToCopy)
            return;

        SerializedEntityClipboard clipboard;
        clipboard.name = entityToCopy->GetName();

        for (auto &component : entityToCopy->GetComponents())
        {
            if (!component)
                continue;

            SerializedComponent serialized;

            std::string componentName = typeid(*component).name();
            componentName = std::regex_replace(componentName, std::regex("class "), "");

            serialized.SetType(componentName);
            serialized.SetGUID(component->GetGUID());

            for (auto &field : component->GetSerializedFields())
            {
                if (!field)
                    continue;

                serialized.AddSerializedField(field->GetName(), field->Serialize());
            }

            clipboard.components.push_back(std::move(serialized));
        }

        EntityClipboard::get().Copy(clipboard);
    }
    void Undo() override
    {
        EntityClipboard::get().Clear();
    }

    std::string GetDescription() const override
    {
        return "Copy Entity: " + entityToCopy->GetName();
    }

private:
    Entity *entityToCopy;
};
