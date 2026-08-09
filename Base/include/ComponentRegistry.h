#pragma once

#include <functional>
#include <string>
#include <type_traits>
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
    std::function<void(Entity *)> addDefault;
    std::function<void(Entity *, ReadableSerializableVariableMap, std::string)> applySerialized;
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
    bool Register(const std::string &name, bool allowsMultiple = false)
    {
        static_assert(std::is_base_of<SerializableScript, T>::value, "Registered type must inherit from SerializableScript");

        if (descriptors.count(name) > 0)
        {
            return false;
        }

        ComponentDescriptor descriptor;
        descriptor.name = name;
        descriptor.allowsMultiple = allowsMultiple;

        descriptor.addDefault = [allowsMultiple](Entity *entity)
        {
            if (entity == nullptr)
            {
                return;
            }

            if (!allowsMultiple && entity->HasComponent<T>())
            {
                return;
            }

            entity->AddComponent<T>();
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

        it->second.addDefault(entity);
        return true;
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
    std::unordered_map<std::string, ComponentDescriptor> descriptors;
    std::vector<std::string> insertionOrder;
};

#define REGISTER_SERIALIZABLE_COMPONENT(TYPE, ALLOWS_MULTIPLE)                              \
    namespace                                                                                \
    {                                                                                        \
        inline const bool TYPE##_component_registered =                                     \
            ComponentRegistry::get().Register<TYPE>(#TYPE, ALLOWS_MULTIPLE);                \
    }
