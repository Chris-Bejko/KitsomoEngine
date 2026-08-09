#pragma once

#include <tuple>

#include "ComponentRegistry.h"

using RegisterProjectComponentFn = void (*)(const char *, bool, ComponentAddDefaultFn, ComponentApplySerializedFn);
using RegisterProjectComponentsExport = void (*)(RegisterProjectComponentFn);

template <typename RequiredTuple, std::size_t... I>
bool ProjectModuleHasRequiredTypesImpl(Entity *entity, std::index_sequence<I...>)
{
    return (entity->HasComponentOfType<std::tuple_element_t<I, RequiredTuple>>() && ...);
}

template <typename T>
bool ProjectModuleHasRequiredTypes(Entity *entity)
{
    using Required = typename ComponentRegistrationMeta<T>::requiredTypes;
    if constexpr (std::tuple_size_v<Required> == 0)
    {
        return true;
    }
    else
    {
        return ProjectModuleHasRequiredTypesImpl<Required>(entity, std::make_index_sequence<std::tuple_size_v<Required>>{});
    }
}

template <typename T>
bool ProjectModuleAddDefault(Entity* entity)
{
    LOG_INFO("========================================");
    LOG_INFO("Adding project component: ", typeid(T).name());
 
    if (entity == nullptr)
    {
        LOG_ERROR("Entity is null");
        return false;
    }

    constexpr bool allowsMultiple =
        ComponentRegistrationMeta<T>::allowMultiple;

    bool hasComponent = entity->HasComponent<T>();
    LOG_INFO(
        "Already has component: ",
        hasComponent ? "YES" : "NO");

    if (!ProjectModuleHasRequiredTypes<T>(entity))
    {
        LOG_ERROR("Required component types missing");
        return false;
    }

    if (!allowsMultiple && entity->HasComponent<T>())
    {
        LOG_ERROR("Component already exists");
        return false;
    }

    LOG_INFO("Calling AddComponent<T>()...");

    entity->AddComponent<T>();

    LOG_INFO(
        "After AddComponent<T>(): ",
        entity->HasComponent<T>() ? "FOUND" : "NOT FOUND");

    if (!entity->HasComponent<T>())
    {
        LOG_ERROR("Component was NOT actually added");
        return false;
    }

    LOG_INFO("Project component successfully added");
    LOG_INFO("========================================");

    return true;
}

template <typename T>
void ProjectModuleApplySerialized(Entity *entity, const ReadableSerializableVariableMap &fields, const char *guid)
{
    if (entity == nullptr)
    {
        return;
    }

    const std::string componentGuid = guid == nullptr ? std::string() : std::string(guid);
    constexpr bool allowsMultiple = ComponentRegistrationMeta<T>::allowMultiple;

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

template <typename T>
void RegisterProjectComponent(
    RegisterProjectComponentFn registerFn,
    const char* name)
{
    registerFn(
        name,
        ComponentRegistrationMeta<T>::allowMultiple,
        &ProjectModuleAddDefault<T>,
        &ProjectModuleApplySerialized<T>
    );

    ComponentRegistry::get()
        .AssociateProjectType<T>(name);
}