#pragma once

#include <string>
#include <typeindex>
#include <unordered_map>

#include "ECS.h"

class ComponentTypeRegistry
{
public:

    static ComponentTypeRegistry& get();


    // --------------------------------------------------------
    // Name -> ID
    // --------------------------------------------------------

    ComponentID GetOrCreateID(
        const std::string& name);


    ComponentID GetID(
        const std::string& name) const;


    // --------------------------------------------------------
    // C++ type -> ID
    // --------------------------------------------------------

    template<typename T>
    ComponentID GetID() const
    {
        auto it =
            typeIDs.find(
                std::type_index(typeid(T)));

        if (it == typeIDs.end())
        {
            return INVALID_COMPONENT_ID;
        }

        return it->second;
    }


    // --------------------------------------------------------
    // Associate C++ type -> existing ID
    // --------------------------------------------------------

    template<typename T>
    void AssociateType(ComponentID id)
    {
        typeIDs[
            std::type_index(typeid(T))
        ] = id;
    }


    // --------------------------------------------------------
    // Register a C++ component type
    //
    // name -> ID
    // T    -> same ID
    // --------------------------------------------------------

    template<typename T>
    ComponentID RegisterType(
        const std::string& name)
    {
        const ComponentID id =
            GetOrCreateID(name);

        AssociateType<T>(id);

        return id;
    }


private:

    ComponentTypeRegistry() = default;


    std::unordered_map<
        std::string,
        ComponentID
    > nameToID;


    std::unordered_map<
        std::type_index,
        ComponentID
    > typeIDs;


    ComponentID nextID = 0;
};