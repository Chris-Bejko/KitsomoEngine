#include "SerializableScript.h"
#include "Engine.h"
#include "Entity.h"
#include "EntityManager.h"

void SerializableScript::ResolvePointers()
{
    // Resolve Entity* fields
    for (auto &[name, field] : entityPtrFields)
    {
        if (field.guidStorage.empty())
            continue;
        for (auto &e : Engine::get().GetManager()->GetEntities())
        {
            if (e->GetGUID() == field.guidStorage)
            {
                *field.ptr = e.get();
                break;
            }
        }
    }

    // Resolve Component* fields
    for (auto &[name, field] : componentPtrFields)
    {
        if (field.guidStorage.empty())
            continue;
        auto pipe = field.guidStorage.find('|');
        if (pipe == std::string::npos)
            continue;

        std::string entityGUID = field.guidStorage.substr(0, pipe);
        std::string compGUID = field.guidStorage.substr(pipe + 1);

        for (auto &e : Engine::get().GetManager()->GetEntities())
        {
            if (e->GetGUID() != entityGUID)
                continue;
            for (auto &comp : e->GetComponents())
            {
                if (comp->GetGUID() == compGUID)
                {
                    *field.ptr = comp.get();
                    break;
                }
            }
        }
    }
}

void SerializableScript::Field(const char *name, Entity *&ptr)
{
    entityPtrFields[name] = {&ptr, ""};
    // Store guid internally
    serializables.push_back({name, &entityPtrFields[name].guidStorage, entityRef_Type});
}

void SerializableScript::Serialize()
{
    // Pack Entity* back to GUID
    for (auto &[name, field] : entityPtrFields)
    {
        if (*field.ptr)
            field.guidStorage = (*field.ptr)->GetGUID();
    }

    // Pack Component* back to GUID
    for (auto &[name, field] : componentPtrFields)
    {
        if (*field.ptr)
        {
            field.guidStorage = (*field.ptr)->entity->GetGUID() +
                                "|" + (*field.ptr)->GetGUID();
        }
    }
}