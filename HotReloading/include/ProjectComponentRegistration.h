
#pragma once

#include "Entity.h"
#include "ComponentRegistry.h"
#include "SerializableScript.h"

// ============================================================
// Add default
// ============================================================

template<typename T>
bool ProjectModuleAddDefault(
    Entity* entity)
{
    if (entity == nullptr)
    {
        return false;
    }


    constexpr bool allowsMultiple =
        ComponentRegistrationMeta<T>::allowMultiple;


    if (!ProjectModuleHasRequiredTypes<T>(
            entity))
    {
        return false;
    }


    if (!allowsMultiple &&
        entity->HasComponent<T>())
    {
        return false;
    }


    entity->AddComponent<T>();

    return entity->HasComponent<T>();
}


// ============================================================
// Apply serialized data
// ============================================================

template<typename T>
void ProjectModuleApplySerialized(
    Entity* entity,
    const ReadableSerializableVariableMap& fields,
    const char* guid)
{
    if (entity == nullptr)
    {
        return;
    }


    const std::string componentGuid =
        guid == nullptr
            ? std::string()
            : std::string(guid);


    constexpr bool allowsMultiple =
        ComponentRegistrationMeta<T>::allowMultiple;


    if (allowsMultiple)
    {
        if (!componentGuid.empty() && entity->HasComponent<T>(componentGuid))
        {
            entity->GetComponent<T>(componentGuid).SetGUID(componentGuid);

            entity->GetComponent<T>(componentGuid).InitSerializedFields(fields);
        }
        else
        {
            entity->AddComponent<T>(FromGUID(componentGuid)).InitSerializedFields(fields);
        }

        return;
    }


    if (!entity->HasComponent<T>())
    {
        entity->AddComponent<T>(FromGUID(componentGuid)).InitSerializedFields(fields);
    }
    else
    {
        if (!componentGuid.empty())
        {
            entity->GetComponent<T>().SetGUID(componentGuid);
        }

        entity->GetComponent<T>().InitSerializedFields(fields);
    }
}