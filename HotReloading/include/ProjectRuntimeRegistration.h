#pragma once

#include "ComponentRegistry.h"
#include "ComponentTypeRegistry.h"

template<typename T>
void RegisterProjectComponentRuntime(
    const char* name)
{
    ComponentID id =
        ComponentTypeRegistry::get().GetOrCreateID(name);

    ComponentTypeRegistry::get().AssociateType<T>(id);

    ComponentRegistry::get().RegisterExternal(
        name,
        ComponentRegistrationMeta<T>::allowMultiple,
        &ProjectModuleAddDefault<T>,
        &ProjectModuleApplySerialized<T>,
        true);
}