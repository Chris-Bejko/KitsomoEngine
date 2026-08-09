#pragma once

#include <string>
#include <unordered_map>

#include "ECS.h"

class ComponentTypeRegistry
{
public:

    static ComponentTypeRegistry& get();

    // ------------------------------------------------------------
    // Name -> ID
    // ------------------------------------------------------------

    ComponentID GetOrCreateID(
        const std::string& name);

    ComponentID GetID(
        const std::string& name) const;


    // ------------------------------------------------------------
    // C++ type -> ID
    //
    // IMPORTANT:
    // This is stored as a static variable belonging to T.
    //
    // We do NOT store std::type_index in the engine.
    // ------------------------------------------------------------

    template<typename T>
    static ComponentID& TypeID()
    {
        static ComponentID id = INVALID_COMPONENT_ID;
        return id;
    }


    template<typename T>
    ComponentID GetID() const
    {
        return TypeID<T>();
    }


    // ------------------------------------------------------------
    // Register a type
    // ------------------------------------------------------------

    template<typename T>
    ComponentID RegisterType(
        const std::string& name)
    {
        const ComponentID id =
            GetOrCreateID(name);

        TypeID<T>() = id;

        return id;
    }


    // ------------------------------------------------------------
    // Associate an already-known project type with its ID
    // ------------------------------------------------------------

    template<typename T>
    void AssociateType(
        ComponentID id)
    {
        TypeID<T>() = id;
    }


private:

    ComponentTypeRegistry() = default;

    std::unordered_map<
        std::string,
        ComponentID
    > nameToID;

    ComponentID nextID = 0;
};