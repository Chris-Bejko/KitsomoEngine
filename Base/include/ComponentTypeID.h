#pragma once

#include "ComponentTypeRegistry.h"

template<typename T>
inline ComponentID getComponentTypeID() noexcept
{
    return ComponentTypeRegistry::get().GetID<T>();
}