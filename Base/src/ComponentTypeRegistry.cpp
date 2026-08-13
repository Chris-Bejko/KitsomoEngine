#include "ComponentTypeRegistry.h"

ComponentTypeRegistry&
ComponentTypeRegistry::get()
{
    static ComponentTypeRegistry instance;
    return instance;
}


ComponentID
ComponentTypeRegistry::GetOrCreateID(
    const std::string& name)
{
    auto it = nameToID.find(name);

    if (it != nameToID.end())
    {
        return it->second;
    }

    if (nextID >= maxComponents)
    {
        return INVALID_COMPONENT_ID;
    }

    ComponentID id = nextID++;

    nameToID[name] = id;

    return id;
}


ComponentID
ComponentTypeRegistry::GetID(
    const std::string& name) const
{
    auto it = nameToID.find(name);

    if (it == nameToID.end())
    {
        return INVALID_COMPONENT_ID;
    }

    return it->second;
}


void
ComponentTypeRegistry::UnregisterProjectTypes(
    const std::vector<std::type_index>& types)
{
    for (const auto& type : types)
    {
        typeIDs.erase(type);
    }
}