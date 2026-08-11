#pragma once

#include <string>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include "ECS.h"

class ComponentTypeRegistry
{
public:

    static ComponentTypeRegistry& get();

    // ------------------------------------------------------------
    // Name -> ID
    // ------------------------------------------------------------

    ComponentID GetOrCreateID(const std::string& name);

    ComponentID GetID(const std::string& name) const;


    // ------------------------------------------------------------
    // C++ type -> ID
    // ------------------------------------------------------------

    template<typename T>
    ComponentID GetID() const
    {
        auto it = typeIDs.find(std::type_index(typeid(T)));

        if (it == typeIDs.end())
        {
            return INVALID_COMPONENT_ID;
        }

        return it->second;
    }


    // ------------------------------------------------------------
    // Register C++ type
    // ------------------------------------------------------------

    template<typename T>
    ComponentID RegisterType(const std::string& name)
    {
        ComponentID id = GetOrCreateID(name);

        typeIDs[std::type_index(typeid(T))] = id;

        return id;
    }


    // ------------------------------------------------------------
    // Associate an already-created ID with a C++ type
    // ------------------------------------------------------------

    template<typename T>
    void AssociateType(ComponentID id)
    {
        if (id == INVALID_COMPONENT_ID)
        {
            return;
        }

        typeIDs[std::type_index(typeid(T))] = id;
    }


    // ------------------------------------------------------------
    // Remove all project DLL types before unloading DLL
    // ------------------------------------------------------------

    void UnregisterProjectTypes(
        const std::vector<std::type_index>& types);


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


// ============================================================
// Global type lookup used by Entity
// ============================================================

template<typename T>
ComponentID getComponentTypeID() noexcept
{
    return ComponentTypeRegistry::get().GetID<T>();
}


// ============================================================
// Engine component registration
//
// REGISTER_COMPONENT(Sprite)
// REGISTER_COMPONENT(Transform)
//
// This registers the component with BOTH:
//   - name -> ID
//   - C++ type -> ID
//
// It does NOT make the component serializable.
// ============================================================

#define REGISTER_COMPONENT(TYPE)                                      \
namespace                                                             \
{                                                                     \
    inline const ComponentID TYPE##_component_registered =            \
        ComponentTypeRegistry::get().RegisterType<TYPE>(#TYPE);       \
}
