#pragma once
#include "Component.h"
#include "Logger.h"
#include "Vector2.h"
#include <functional>
#include <sstream>
#include <regex>
#include "Texture.h"
#include "Audio.h"
struct EntityPtrField
{
    Entity **ptr;
    std::string guidStorage;
};

struct ComponentPtrField
{
    Component **ptr;
    std::string guidStorage;
    std::string compType;
};

struct VectorFieldEntry
{
    std::function<void()> serialize;
    std::function<void(const std::string &)> deserialize;
    int elementType = char_Type; // actual element type
};

struct VectorPtrFieldEntry
{
    std::function<void()> serialize;
    std::function<void(const std::string &)> resolve;
    int elementType = entityRef_Type; // entityRef_Type or compRef_Type
};

class SerializableScript : public Component
{
    friend class Entity;

public:
    // Primitive fields
    void Field(const char *name, int &val) { serializables.push_back({name, &val, int_Type}); }
    void Field(const char *name, float &val) { serializables.push_back({name, &val, float_Type}); }
    void Field(const char *name, std::string &val) { serializables.push_back({name, &val, char_Type}); }
    void Field(const char *name, bool &val) { serializables.push_back({name, &val, bool_Type}); }
    void Field(const char *name, unsigned int &val) { serializables.push_back({name, &val, int_Type}); }
    template <typename T, typename Derived>
    void Field(const char *name, VectorBase<T, Derived> &val)
    {
        serializables.push_back({
            name,
            &val,
            mathVector_Type,
            "",
            (int)Derived::Size,
            std::is_integral<T>::value // isInt flag
        });
    }

    void Field(const char* name, AssetReference& asset)
    {
        assetFields[name] = &asset;
        assetStrings[name] = asset.GetName();
        serializables.push_back({name, &assetStrings[name], file_Type});
    }

    // Entity* field
    void Field(const char *name, Entity *&ptr);

    // Component* field
    template <typename T>
    void Field(const char *name, T *&ptr)
    {
        std::string typeName = typeid(T).name();
        typeName = std::regex_replace(typeName, std::regex("class "), "");
        componentPtrFields[name] = {reinterpret_cast<Component **>(&ptr), "", typeName};
        serializables.push_back({name, &componentPtrFields[name].guidStorage, compRef_Type, typeName});
    }

    // Element serialization
    template <typename T>
    std::string SerializeElement(const T &val) { return std::to_string(val); }
    template <typename T>
    T DeserializeElement(const std::string &s);

    template <>
    inline std::string SerializeElement(const float &v) { return std::to_string(v); }
    template <>
    inline std::string SerializeElement(const int &v) { return std::to_string(v); }
    template <>
    inline std::string SerializeElement(const std::string &v) { return v; }
    template <>
    inline std::string SerializeElement(const bool &v) { return v ? "1" : "0"; }
    template <>
    inline std::string SerializeElement(const Vector2F &v)
    {
        return std::to_string(v.x) + "|" + std::to_string(v.y);
    }

    template <>
    inline float DeserializeElement<float>(const std::string &s) { return std::stof(s); }
    template <>
    inline int DeserializeElement<int>(const std::string &s) { return std::stoi(s); }
    template <>
    inline std::string DeserializeElement<std::string>(const std::string &s) { return s; }
    template <>
    inline bool DeserializeElement<bool>(const std::string &s) { return s == "1"; }
    template <>
    inline Vector2F DeserializeElement<Vector2F>(const std::string &s)
    {
        auto pipe = s.find('|');
        if (pipe == std::string::npos)
            return {};
        return {std::stof(s.substr(0, pipe)), std::stof(s.substr(pipe + 1))};
    }

    // Vector of primitives
    template <typename T>
    std::string SerializeVector(const std::vector<T> &vec)
    {
        std::string result;
        for (size_t i = 0; i < vec.size(); i++)
        {
            result += SerializeElement(vec[i]);
            if (i < vec.size() - 1)
                result += ";";
        }
        return result;
    }

    template <typename T>
    void DeserializeVector(const std::string &s, std::vector<T> &vec)
    {
        vec.clear();
        if (s.empty())
            return;
        std::stringstream ss(s);
        std::string token;
        while (std::getline(ss, token, ';'))
        {
            if (token.empty())
                continue;
            try
            {
                vec.push_back(DeserializeElement<T>(token));
            }
            catch (...)
            {
                LOG_WARNING("Failed to deserialize: ", token.c_str());
            }
        }
    }

    // Field for vector of primitives
    template <typename T>
    void Field(const char *name, std::vector<T> &vec)
    {
        vectorStrings[name] = SerializeVector(vec);
        serializables.push_back({name, &vectorStrings[name], char_Type});
        vectorFields[name] = {
            [&vec, name, this]()
            { vectorStrings[name] = SerializeVector(vec); },
            [&vec, this](const std::string &s)
            { DeserializeVector(s, vec); },
            // Deduce element type from T
            std::is_same<T, int>::value ? int_Type : std::is_same<T, float>::value ? float_Type
                                                 : std::is_same<T, bool>::value    ? bool_Type
                                                                                   : char_Type};
    }

    // Field for vector of Entity*
    void Field(const char *name, std::vector<Entity *> &vec);

    // Field for vector of Component*
    template <typename T>
    void Field(const char *name, std::vector<T *> &vec)
    {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
        std::string typeName = typeid(T).name();
        typeName = std::regex_replace(typeName, std::regex("class "), "");

        vectorStrings[name] = "";
        serializables.push_back({name, &vectorStrings[name], compRef_Type, typeName});

        vectorPtrFields[name] = {
            [&vec, name, this]()
            {
                std::string result;
                for (size_t i = 0; i < vec.size(); i++)
                {
                    if (vec[i])
                        result += vec[i]->entity->GetGUID() + "|" + vec[i]->GetGUID();
                    if (i < vec.size() - 1)
                        result += ";";
                }
                vectorStrings[name] = result;
            },
            [&vec, this](const std::string &s)
            {
                vec.clear();
                if (s.empty())
                    return;
                std::stringstream ss(s);
                std::string token;
                while (std::getline(ss, token, ';'))
                {
                    if (token.empty())
                        continue;
                    auto pipe = token.find('|');
                    if (pipe == std::string::npos)
                        continue;
                    std::string entityGUID = token.substr(0, pipe);
                    std::string compGUID = token.substr(pipe + 1);
                    for (auto &e : Engine::get().GetManager()->GetEntities())
                    {
                        if (e->GetGUID() != entityGUID)
                            continue;
                        for (auto &comp : e->GetComponents())
                        {
                            if (comp->GetGUID() == compGUID)
                            {
                                T *typed = dynamic_cast<T *>(comp.get());
                                if (typed)
                                    vec.push_back(typed);
                                break;
                            }
                        }
                    }
                }
            },
            compRef_Type};
    }

    template <typename T>
    T *FindObjectOfType()
    {
        for (auto &e : Engine::get().GetManager()->GetEntities())
        {
            if (e->HasComponent<T>())
            {
                T *found = &e->GetComponent<T>();
                SyncPtrToGUID(found, e.get());
                return found;
            }
        }
        return nullptr;
    }

    template <typename T>
    void SyncPtrToGUID(T *comp, Entity *owner)
    {
        for (auto &[name, field] : componentPtrFields)
        {
            if (*field.ptr == static_cast<Component *>(comp))
            {
                field.guidStorage = owner->GetGUID() + "|" + comp->GetGUID();
                return;
            }
        }
    }

    bool IsVectorField(const std::string &name)
    {
        return vectorFields.count(name) || vectorPtrFields.count(name);
    }
    bool IsVectorPtrField(const std::string &name)
    {
        return vectorPtrFields.count(name);
    }

    void Serialize() override;
    void InitSerializedFields(ReadableSerializableVariableMap map) override;
    void ResolvePointers() override;
    std::vector<SerializableVariable> *GetSerializedFields() override { return &serializables; }
    virtual void OnFieldChanged(const std::string &fieldName) {}
    void NotifyFieldChanged(const std::string &fieldName) { OnFieldChanged(fieldName); }
    virtual bool Init() override { return true; }
    AssetReference* GetAssetReference(const std::string& fieldName)
    {
        auto it = assetFields.find(fieldName);

        if (it == assetFields.end())
            return nullptr;

        return it->second;
    }
protected:
    std::vector<SerializableVariable> serializables;
    std::map<std::string, EntityPtrField> entityPtrFields;
    std::map<std::string, ComponentPtrField> componentPtrFields;
    std::map<std::string, std::string> vectorStrings;
    std::map<std::string, VectorFieldEntry> vectorFields;
    std::map<std::string, VectorPtrFieldEntry> vectorPtrFields;
    std::map<std::string, std::string> assetStrings;
    std::unordered_map<std::string, AssetReference*> assetFields;
    bool fieldsRegistered = false;
    std::vector<std::string> fieldNameStorage; // keeps names alive

    void *GetFieldData(const std::string &name)
    {
        for (auto &var : serializables)
            if (std::string(var.name) == name)
                return var.data;
        return nullptr;
    }
};