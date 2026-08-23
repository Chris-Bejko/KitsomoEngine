
#pragma once

#include "Entity.h"
#include "ComponentRegistry.h"
#include "SerializableScript.h"

// ============================================================
// Add default
// ============================================================

template <typename T>
bool ProjectModuleAddDefault(
    Entity *entity)
{
    if (entity == nullptr)
    {
        return false;
    }

    constexpr bool allowsMultiple =
        ComponentRegistrationMeta<T>::allowMultiple;

    if (!ProjectModuleHasRequiredTypes<T>(entity))
    {
        return false;
    }

    if (!allowsMultiple && entity->HasComponent<T>())
    {
        return false;
    }

    entity->AddComponent<T>();

    return entity->HasComponent<T>();
}

// ============================================================
// Apply serialized data
// ============================================================

template <typename T>
void ProjectModuleApplySerialized(Entity *entity, const SerializedComponent &serialized)
{
    if (!entity)
    {
        LOG_ERROR("ProjectModuleApplySerialized: entity is null");
        return;
    }

    T *component = nullptr;

    const std::string componentGuid = serialized.GetGUID();

    constexpr bool allowsMultiple = ComponentRegistrationMeta<T>::allowMultiple;

    if (allowsMultiple)
    {
        if (!componentGuid.empty() && entity->HasComponent<T>(componentGuid))
        {
            component = &entity->GetComponent<T>(componentGuid);
        }
        else
        {
            component = &entity->AddComponent<T>(FromGUID(componentGuid));
        }

        return;
    }
    else
    {
        if (!entity->HasComponent<T>())
        {
            component = &entity->AddComponent<T>(FromGUID(componentGuid));
        }
        else
        {
            component = &entity->GetComponent<T>();
        }
    }

    if (!component)
    {
        LOG_ERROR("ProjectModuleApplySerialized: failed to create/find component");
        return;
    }

    for (const auto &field : component->GetSerializedFields())
    {
        if (!field)
            continue;
        auto it = serialized.GetFields().find(field->GetName());
        if (it != serialized.GetFields().end())
        {
            field->Deserialize(it->second);
        }
    }
}