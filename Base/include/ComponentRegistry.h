#pragma once

#include <functional>
#include <string>
#include <type_traits>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Entity.h"
#include "GUIDGenerator.h"
#include "SerializableScript.h"

struct ComponentDescriptor
{
    std::string name;
    bool allowsMultiple = false;
    std::function<bool(Entity *)> addDefault;
    std::function<void(Entity *, ReadableSerializableVariableMap, std::string)> applySerialized;
};

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

    template <typename T>
    bool Register(const std::string &name)
    {
        static_assert(std::is_base_of<SerializableScript, T>::value, "Registered type must inherit from SerializableScript");

        if (descriptors.count(name) > 0)
        {
            return false;
        }

        ComponentDescriptor descriptor;
        descriptor.name = name;
        constexpr bool allowsMultiple = ComponentRegistrationMeta<T>::allowMultiple;
        descriptor.allowsMultiple = allowsMultiple;

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
            return true;
        };

        descriptor.applySerialized = [allowsMultiple](Entity *entity, ReadableSerializableVariableMap fields, std::string guid)
        {
            if (entity == nullptr)
            {
                return;
            }

            if (allowsMultiple)
            {
                if (!guid.empty() && entity->HasComponent<T>(guid))
                {
                    entity->GetComponent<T>(guid).SetGUID(guid);
                    entity->GetComponent<T>(guid).InitSerializedFields(fields);
                }
                else
                {
                    entity->AddComponent<T>(FromGUID(guid)).InitSerializedFields(fields);
                }
                return;
            }

            if (!entity->HasComponent<T>())
            {
                entity->AddComponent<T>(FromGUID(guid)).InitSerializedFields(fields);
            }
            else
            {
                if (!guid.empty())
                {
                    entity->GetComponent<T>().SetGUID(guid);
                }
                entity->GetComponent<T>().InitSerializedFields(fields);
            }
        };

        descriptors[name] = std::move(descriptor);
        insertionOrder.push_back(name);
        return true;
    }

    bool AddByName(Entity *entity, const std::string &name) const
    {
        auto it = descriptors.find(name);
        if (it == descriptors.end())
        {
            return false;
        }

        return it->second.addDefault(entity);
    }

    bool IsMultiInstance(const std::string &name) const
    {
        auto it = descriptors.find(name);
        if (it == descriptors.end())
        {
            return false;
        }
        return it->second.allowsMultiple;
    }

    const std::unordered_map<std::string, ComponentDescriptor> &GetAll() const
    {
        return descriptors;
    }

    std::vector<std::string> GetNames() const
    {
        return insertionOrder;
    }

private:
    template <typename T>
    static bool HasRequiredTypes(Entity *entity)
    {
        using Required = typename ComponentRegistrationMeta<T>::requiredTypes;
        if constexpr (std::tuple_size_v<Required> == 0)
        {
            return true;
        }
        else
        {
            return HasRequiredTypesImpl<Required>(entity, std::make_index_sequence<std::tuple_size_v<Required>>{});
        }
    }

    template <typename RequiredTuple, std::size_t... I>
    static bool HasRequiredTypesImpl(Entity *entity, std::index_sequence<I...>)
    {
        return (entity->HasComponentOfType<std::tuple_element_t<I, RequiredTuple>>() && ...);
    }

    std::unordered_map<std::string, ComponentDescriptor> descriptors;
    std::vector<std::string> insertionOrder;
};

#define REGISTER_SERIALIZABLE_COMPONENT(TYPE, ...)                                           \
    namespace                                                                                \
    {                                                                                        \
        inline const bool TYPE##_component_registered =                                     \
            ComponentRegistry::get().Register<TYPE>(#TYPE);                                 \
    }

#define DECLARE_COMPONENT_RULES(TYPE, ALLOW_MULTIPLE, ...)                                   \
    template <>                                                                               \
    struct ComponentRegistrationMeta<TYPE>                                                    \
    {                                                                                         \
        static constexpr bool allowMultiple = ALLOW_MULTIPLE;                                 \
        using requiredTypes = std::tuple<__VA_ARGS__>;                                        \
    };

