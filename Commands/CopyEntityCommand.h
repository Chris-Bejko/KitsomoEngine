#pragma once
#include "Command.h"
#include "EntityClipboard.h"
#include "Entity.h"

class CopyEntityCommand : public Command
{
public:
    CopyEntityCommand(Entity *entity) : entityToCopy(entity)
    {
    }

    void Execute() override
    {
        if (entityToCopy)
        {
            SerializedEntity clipboard;
            clipboard.name = entityToCopy->GetName();
            clipboard.components = entityToCopy->GetAllComponentVariables(); // already has fields built!
            EntityClipboard::get().Copy(clipboard);
        }
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
