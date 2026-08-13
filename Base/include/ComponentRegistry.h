#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "Entity.h"
#include "GUIDGenerator.h"
#include "SerializableScript.h"
#include "ComponentTypeRegistry.h"

struct ComponentDescriptor
{
    std::string name;

    bool allowsMultiple = false;
    bool isProjectComponent = false;

    std::function<bool(Entity*)> addDefault;

    std::function<
        void(
            Entity*,
            ReadableSerializableVariableMap,
            std::string
        )
    > applySerialized;
};


using ComponentAddDefaultFn =
    bool (*)(Entity*);


using ComponentApplySerializedFn =
    void (*)(
        Entity*,
        const ReadableSerializableVariableMap&,
        const char*
    );


template<typename T>
struct ComponentRegistrationMeta
{
    static constexpr bool allowMultiple = false;

    using requiredTypes = std::tuple<>;
};


class ComponentRegistry
{
public:

    static ComponentRegistry& get()
    {
        static ComponentRegistry instance;
        return instance;
    }


    // ========================================================
    // Register normal engine serializable component
    // ========================================================

   template<typename T>
bool Register(const std::string& name)
{
    static_assert(
        std::is_base_of<SerializableScript, T>::value,
        "Registered type must inherit from SerializableScript"
    );

    if (descriptors.count(name) > 0)
    {
        return false;
    }

    // --------------------------------------------------------
    // ECS registration
    // --------------------------------------------------------

    ComponentTypeRegistry::get().RegisterType<T>(name);

    // --------------------------------------------------------
    // Serializable registry registration
    // --------------------------------------------------------

    ComponentDescriptor descriptor;

    descriptor.name = name;

    constexpr bool allowsMultiple =
        ComponentRegistrationMeta<T>::allowMultiple;

    descriptor.allowsMultiple = allowsMultiple;
    descriptor.isProjectComponent = false;

    descriptor.addDefault =
        [allowsMultiple](Entity* entity)
    {
        if (entity == nullptr)
        {
            return false;
        }

        if (!HasRequiredTypes<T>(entity))
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
    };

    descriptor.applySerialized =
        [allowsMultiple](
            Entity* entity,
            ReadableSerializableVariableMap fields,
            std::string guid)
    {
        if (entity == nullptr)
        {
            return;
        }

        if (allowsMultiple)
        {
            if (!guid.empty() &&
                entity->HasComponent<T>(guid))
            {
                entity
                    ->GetComponent<T>(guid)
                    .SetGUID(guid);

                entity
                    ->GetComponent<T>(guid)
                    .InitSerializedFields(fields);
            }
            else
            {
                entity
                    ->AddComponent<T>(FromGUID(guid))
                    .InitSerializedFields(fields);
            }

            return;
        }

        if (!entity->HasComponent<T>())
        {
            entity
                ->AddComponent<T>(FromGUID(guid))
                .InitSerializedFields(fields);
        }
        else
        {
            if (!guid.empty())
            {
                entity
                    ->GetComponent<T>()
                    .SetGUID(guid);
            }

            entity
                ->GetComponent<T>()
                .InitSerializedFields(fields);
        }
    };

    descriptors[name] =
        std::move(descriptor);

    insertionOrder.push_back(name);

    return true;
}


    // ========================================================
    // Register project component
    // ========================================================

    bool RegisterExternal(
        const std::string& name,
        bool allowsMultiple,
        ComponentAddDefaultFn addDefault,
        ComponentApplySerializedFn applySerialized,
        bool isProjectComponent)
    {
        if (name.empty() ||
            addDefault == nullptr ||
            applySerialized == nullptr)
        {
            return false;
        }

        if (descriptors.count(name) > 0)
        {
            return false;
        }


        ComponentDescriptor descriptor;

        descriptor.name =
            name;

        descriptor.allowsMultiple =
            allowsMultiple;

        descriptor.isProjectComponent =
            isProjectComponent;


        descriptor.addDefault =
            [addDefault](Entity* entity)
        {
            return addDefault(entity);
        };


        descriptor.applySerialized =
            [applySerialized](
                Entity* entity,
                ReadableSerializableVariableMap fields,
                std::string guid)
        {
            applySerialized(
                entity,
                fields,
                guid.c_str()
            );
        };


        descriptors[name] =
            std::move(descriptor);

        insertionOrder.push_back(name);

        return true;
    }


    // ========================================================
    // Add component by name
    // ========================================================

    bool AddByName(
        Entity* entity,
        const std::string& name) const
    {
        auto it =
            descriptors.find(name);

        if (it == descriptors.end())
        {
            return false;
        }

        return it->second.addDefault(entity);
    }


    // ========================================================
    // Multi-instance
    // ========================================================

    bool IsMultiInstance(
        const std::string& name) const
    {
        auto it =
            descriptors.find(name);

        if (it == descriptors.end())
        {
            return false;
        }

        return it->second.allowsMultiple;
    }


    // ========================================================
    // All descriptors
    // ========================================================

    const std::unordered_map<
        std::string,
        ComponentDescriptor
    >& GetAll() const
    {
        return descriptors;
    }


    // ========================================================
    // Unregister one
    // ========================================================

    void Unregister(
        const std::string& name)
    {
        descriptors.erase(name);

        insertionOrder.erase(
            std::remove(
                insertionOrder.begin(),
                insertionOrder.end(),
                name
            ),
            insertionOrder.end()
        );
    }


    // ========================================================
    // Remove project components before DLL unload
    // ========================================================

    void UnregisterProjectComponents()
    {
        std::vector<std::string> namesToRemove;

        for (const auto& entry : descriptors)
        {
            if (entry.second.isProjectComponent)
            {
                namesToRemove.push_back(
                    entry.first
                );
            }
        }

        for (const auto& name : namesToRemove)
        {
            Unregister(name);
        }
    }


    // ========================================================
    // Names
    // ========================================================

    std::vector<std::string> GetNames() const
    {
        return insertionOrder;
    }


private:

    template<typename T>
    static bool HasRequiredTypes(
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
            return HasRequiredTypesImpl<T>(
                entity,
                std::make_index_sequence<
                    std::tuple_size_v<Required>
                >{}
            );
        }
    }


    template<
        typename T,
        std::size_t... I
    >
    static bool HasRequiredTypesImpl(
        Entity* entity,
        std::index_sequence<I...>)
    {
        using Required =
            typename ComponentRegistrationMeta<T>::requiredTypes;

        return (
            entity->HasComponentOfType<
                std::tuple_element_t<
                    I,
                    Required
                >
            >()
            && ...
        );
    }


    std::unordered_map<
        std::string,
        ComponentDescriptor
    > descriptors;

    std::vector<std::string>
        insertionOrder;
};


// ============================================================
// Serializable engine component macro
//
// This registers ONLY with ComponentRegistry.
//
// It does NOT create the ECS ID.
// ============================================================

#ifdef ECS_PROJECT_MODULE_BUILD

#define REGISTER_SERIALIZABLE_COMPONENT(TYPE, ...)

#else

#define REGISTER_SERIALIZABLE_COMPONENT(TYPE)                         \
namespace                                                             \
{                                                                     \
    inline const bool TYPE##_serializable_registered =                \
        ComponentRegistry::get().Register<TYPE>(#TYPE);               \
}
#endif


// ============================================================
// Component rules
// ============================================================

#define DECLARE_COMPONENT_RULES(                                      \
    TYPE,                                                            \
    ALLOW_MULTIPLE,                                                  \
    ...)                                                              \
    template<>                                                        \
    struct ComponentRegistrationMeta<TYPE>                            \
    {                                                                 \
        static constexpr bool allowMultiple =                         \
            ALLOW_MULTIPLE;                                           \
                                                                        \
        using requiredTypes =                                         \
            std::tuple<__VA_ARGS__>;                                  \
    };