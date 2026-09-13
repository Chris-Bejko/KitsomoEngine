#pragma once
#include "Component.h"
#include "Logger.h"
#include "Vector2.h"
#include <functional>
#include <sstream>
#include <regex>
#include "Texture.h"
#include "Audio.h"
#include "MathVectorField.h"
#include "DynamicVectorField.h"
#include "ComponentRefField.h"
#include "ComponentVectorField.h"
class SerializableScript : public Component
{
    friend class Entity;

public:
    // Primitive fields
    void Field(const char *name, int &val);
    void Field(const char *name, float &val);
    void Field(const char *name, std::string &val);
    void Field(const char *name, bool &val);
    void Field(const char *name, unsigned int &val);
    void Field(const char *name, Entity *&ptr);

    template <typename T, typename Derived>
    void Field(const char *name, VectorBase<T, Derived> &val)
    {
        serializedFields.push_back(std::make_unique<MathVectorField<T, Derived>>(name, &val));
    }

    void Field(const char *name, AssetReference &asset);
    // Component* field
    template <typename T>
    void Field(const char *name, T *&ptr)
    {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
        std::string typeName = typeid(T).name();
        typeName = std::regex_replace(typeName, std::regex("class "), "");

        serializedFields.push_back(std::make_unique<ComponentRefField>(name, reinterpret_cast<Component **>(&ptr), typeName));
    }

    template <typename T>
    void Field(const char *name, std::vector<T> &vec)
    {
        serializedFields.push_back(std::make_unique<DynamicVectorField<T>>(name, &vec));
    }

    // Field for vector of Entity*
    void Field(const char *name, std::vector<Entity *> &vec);

    template <typename T>
    void Field(const char *name, std::vector<T *> &vec)
    {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

        std::string typeName = typeid(T).name();

        typeName = std::regex_replace(typeName, std::regex("class "), "");

        serializedFields.push_back(std::make_unique<ComponentVectorField<T>>(name, &vec, typeName));
    }

    template <typename T>
    T *FindObjectOfType()
    {
        for (auto &e : Engine::get().GetManager()->GetEntities())
        {
            if (e->HasComponent<T>())
            {
                T *found = &e->GetComponent<T>();
                return found;
            }
        }
        return nullptr;
    }

    void ResolvePointers() override
    {
        for (auto &field : serializedFields)
        {
            field->Resolve();
        }
    }
    const std::vector<std::unique_ptr<SerializedField>> &GetSerializedFields() const override { return serializedFields; }
    virtual void OnFieldChanged(const std::string &fieldName) {}
    void NotifyFieldChanged(const std::string &fieldName) { OnFieldChanged(fieldName); }
    virtual bool Init() override { return true; }

protected:
    std::vector<std::unique_ptr<SerializedField>> serializedFields;
    bool fieldsRegistered = false;
};