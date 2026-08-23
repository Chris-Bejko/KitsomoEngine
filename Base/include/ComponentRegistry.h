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
#include "SerializedComponent.h"

struct ComponentDescriptor
{
    std::string name;

    bool allowsMultiple = false;
    bool isProjectComponent = false;

    std::function<bool(Entity *)> addDefault;

    std::function<
        void(
            Entity *,
            const SerializedComponent &)>
        factory;
};

using ComponentAddDefaultFn = bool (*)(Entity *);

using ComponentApplySerializedFn = void (*)(Entity *, const SerializedComponent &);
template <typename T>
struct ComponentRegistrationMeta
{
    static constexpr bool allowMultiple = false;

    using requiredTypes = std::tuple<>;
};

class ComponentRegistry
{
public:
    static ComponentRegistry &get()
    {
        static ComponentRegistry instance;
        return instance;
    }

    // ========================================================
    // Register normal engine serializable component
    // ========================================================

    template <typename T>
    bool Register(const std::string &name)
    {
        static_assert(std::is_base_of<SerializableScript, T>::value, "Registered type must inherit from SerializableScript");

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

        constexpr bool allowsMultiple = ComponentRegistrationMeta<T>::allowMultiple;

        descriptor.allowsMultiple = allowsMultiple;
        descriptor.isProjectComponent = false;

        descriptor.addDefault = [allowsMultiple](Entity *entity)
        {
            if (entity == nullptr)
            {
                return false;
            }

            if (!HasRequiredTypes<T>(entity))
            {
                return false;
            }

            if (!allowsMultiple && entity->HasComponent<T>())
            {
                return false;
            }

            entity->AddComponent<T>();

            return entity->HasComponent<T>();
        };

        descriptor.factory = [allowsMultiple](Entity *entity, const SerializedComponent &serialized)
        {
            if (entity == nullptr)
            {
                return;
            }

            const std::string &guid = serialized.GetGUID();
            T *component = nullptr;
            if (allowsMultiple)
            {
                if (!guid.empty() && entity->HasComponent<T>(guid))
                {
                    component = &entity->GetComponent<T>(guid);
                }
                else
                {
                    component = &entity->AddComponent<T>(FromGUID(guid));
                }

                return;
            }

            if (!entity->HasComponent<T>())
            {
                component = &entity->AddComponent<T>(FromGUID(guid));
            }
            else
            {
                component = &entity->GetComponent<T>();
                if (!guid.empty())
                {
                    component->SetGUID(guid);
                }
            }

            for (const auto &fieldPtr : component->GetSerializedFields())
            {
                if (!fieldPtr)
                    continue;

                SerializedField *field = fieldPtr.get();
                auto it = serialized.GetFields().find(field->GetName());
                if (it != serialized.GetFields().end())
                    field->Deserialize(it->second);
            }
        };
        descriptors[name] = std::move(descriptor);

        insertionOrder.push_back(name);

        return true;
    }

    // ========================================================
    // Register project component
    // ========================================================

    bool RegisterExternal(const std::string &name, bool allowsMultiple, ComponentAddDefaultFn addDefault, ComponentApplySerializedFn factory)
    {
        LOG_INFO("RegisterExternal: ", name.c_str());

        if (name.empty() || addDefault == nullptr || factory == nullptr)
        {
            LOG_ERROR("RegisterExternal FAILED validation: ", name.c_str());

            return false;
        }

        if (descriptors.count(name) > 0)
        {
            LOG_ERROR("RegisterExternal FAILED: component already exists: ",name.c_str());

            return false;
        }

        ComponentDescriptor descriptor;

        descriptor.name = name;
        descriptor.allowsMultiple = allowsMultiple;
        descriptor.isProjectComponent = true;

        descriptor.addDefault = [addDefault](Entity *entity)
        {
            return addDefault(entity);
        };

        descriptor.factory = [factory](Entity *entity, const SerializedComponent &serialized)
        {
            factory(
                entity,
                serialized);
        };

        descriptors[name] = std::move(descriptor);

        insertionOrder.push_back(name);

        LOG_INFO("RegisterExternal SUCCESS: ", name.c_str());

        LOG_INFO("Descriptor count now: ", std::to_string(descriptors.size()).c_str());

        return true;
    }

    // ========================================================
    // Add component by name
    // ========================================================

    bool AddByName(Entity *entity, const std::string &name) const
    {
        auto it = descriptors.find(name);

        if (it == descriptors.end())
        {
            return false;
        }

        return it->second.addDefault(entity);
    }

    // ========================================================
    // Multi-instance
    // ========================================================

    bool IsMultiInstance(const std::string &name) const
    {
        auto it = descriptors.find(name);

        if (it == descriptors.end())
        {
            return false;
        }

        return it->second.allowsMultiple;
    }

    // ========================================================
    // All descriptors
    // ========================================================

    const std::unordered_map<std::string, ComponentDescriptor> &
    GetAll() const
    {
        return descriptors;
    }

    // ========================================================
    // Unregister one
    // ========================================================

    void Unregister(const std::string &name)
    {
        descriptors.erase(name);

        insertionOrder.erase(std::remove(insertionOrder.begin(), insertionOrder.end(), name), insertionOrder.end());
    }

    // ========================================================
    // Remove project components before DLL unload
    // ========================================================

    void UnregisterProjectComponents()
    {
        std::vector<std::string> namesToRemove;

        for (const auto &entry : descriptors)
        {
            if (entry.second.isProjectComponent)
            {
                namesToRemove.push_back(
                    entry.first);
            }
        }

        for (const auto &name : namesToRemove)
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
    template <typename T>
    static bool HasRequiredTypes(
        Entity *entity)
    {
        using Required =
            typename ComponentRegistrationMeta<T>::requiredTypes;

        if constexpr (std::tuple_size_v<Required> == 0)
        {
            return true;
        }
        else
        {
            return HasRequiredTypesImpl<T>(
                entity,
                std::make_index_sequence<
                    std::tuple_size_v<Required>>{});
        }
    }

    template <
        typename T,
        std::size_t... I>
    static bool HasRequiredTypesImpl(
        Entity *entity,
        std::index_sequence<I...>)
    {
        using Required =
            typename ComponentRegistrationMeta<T>::requiredTypes;

        return (
            entity->HasComponentOfType<
                std::tuple_element_t<
                    I,
                    Required>>() &&
            ...);
    }

    std::unordered_map<
        std::string,
        ComponentDescriptor>
        descriptors;

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

#define REGISTER_SERIALIZABLE_COMPONENT(TYPE)               \
    namespace                                               \
    {                                                       \
        inline const bool TYPE##_serializable_registered =  \
            ComponentRegistry::get().Register<TYPE>(#TYPE); \
    }
#endif

// ============================================================
// Component rules
// ============================================================

#define DECLARE_COMPONENT_RULES(              \
    TYPE,                                     \
    ALLOW_MULTIPLE,                           \
    ...)                                      \
    template <>                               \
    struct ComponentRegistrationMeta<TYPE>    \
    {                                         \
        static constexpr bool allowMultiple = \
            ALLOW_MULTIPLE;                   \
                                              \
        using requiredTypes =                 \
            std::tuple<__VA_ARGS__>;          \
    };