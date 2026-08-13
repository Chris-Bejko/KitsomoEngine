#pragma once

#include <cstddef>
#include <tuple>

#include "ComponentRegistry.h"
#include "ComponentTypeRegistry.h"


using RegisterProjectComponentFn =
    void (*)(
        const char*,
        bool,
        ComponentAddDefaultFn,
        ComponentApplySerializedFn
    );


using RegisterProjectComponentsExport =
    void (*)(
        RegisterProjectComponentFn
    );


// ============================================================
// Required component checking
// ============================================================

template<
    typename RequiredTuple,
    std::size_t... I
>
bool ProjectModuleHasRequiredTypesImpl(
    Entity* entity,
    std::index_sequence<I...>)
{
    return (
        entity->HasComponentOfType<
            std::tuple_element_t<
                I,
                RequiredTuple
            >
        >()
        && ...
    );
}


template<typename T>
bool ProjectModuleHasRequiredTypes(
    Entity* entity)
{
    using Required =
        typename ComponentRegistrationMeta<T>::requiredTypes;

    if constexpr (
        std::tuple_size_v<Required> == 0)
    {
        return true;
    }
    else
    {
        return ProjectModuleHasRequiredTypesImpl<
            Required
        >(
            entity,
            std::make_index_sequence<
                std::tuple_size_v<Required>
            >{}
        );
    }
}


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


// ============================================================
// Register project component
//
// THIS DOES BOTH:
//
// 1. ComponentTypeRegistry
// 2. ComponentRegistry
//
// So Player becomes both an ECS component and a
// serializable/inspector component.
// ============================================================

template<typename T>
void RegisterProjectComponent(
    RegisterProjectComponentFn registerFn,
    const char* name)
{
    ComponentID id =
        ComponentTypeRegistry::get().GetOrCreateID(name);

    ComponentTypeRegistry::get().AssociateType<T>(id);

    registerFn(
        name,
        ComponentRegistrationMeta<T>::allowMultiple,
        &ProjectModuleAddDefault<T>,
        &ProjectModuleApplySerialized<T>
    );
}