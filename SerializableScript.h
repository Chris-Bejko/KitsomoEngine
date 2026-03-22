// SerializableScript.h
#pragma once
#include "../Component.h"
#include "../Logger.h"

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
class SerializableScript : public Component
{
public:
    void Field(const char *name, int &val) { serializables.push_back({name, &val, int_Type}); }
    void Field(const char *name, float &val) { serializables.push_back({name, &val, float_Type}); }
    void Field(const char *name, std::string &val) { serializables.push_back({name, &val, char_Type}); }
    void Field(const char *name, bool &val) { serializables.push_back({name, &val, bool_Type}); }
    void Field(const char *name, unsigned int val) { serializables.push_back({name, (int *)&val, int_Type}); }
    void Field(const char *name, Entity *&ptr);

    virtual bool Init() override { return true; }

    template <typename T>
    void Field(const char *name, T *&ptr)
    {
        // T is a Component type
        std::string typeName = typeid(T).name();
        typeName = std::regex_replace(typeName, std::regex("class "), "");
        componentPtrFields[name] = {reinterpret_cast<Component **>(&ptr), "", typeName};
        serializables.push_back({name, &componentPtrFields[name].guidStorage, compRef_Type, typeName});
    }

    void Serialize() override;
    void InitSerializedFields(ReadableSerializableVariableMap map) override
    {
        for (auto &var : serializables)
        {
            std::string fieldName(var.name);

            switch (var.type)
            {
            case int_Type:
                if (map.intFields.count(fieldName))
                    *reinterpret_cast<int *>(var.data) = map.intFields[fieldName];
                break;

            case float_Type:
                if (map.floatFields.count(fieldName))
                    *reinterpret_cast<float *>(var.data) = map.floatFields[fieldName];
                break;

            case char_Type:
                if (map.stringFields.count(fieldName))
                    *reinterpret_cast<std::string *>(var.data) = map.stringFields[fieldName];
                break;

            case bool_Type:
                if (map.boolFields.count(fieldName))
                    *reinterpret_cast<bool *>(var.data) = map.boolFields[fieldName];
                // Also handle bool saved as int (0/1)
                else if (map.intFields.count(fieldName))
                    *reinterpret_cast<bool *>(var.data) = map.intFields[fieldName] != 0;
                break;

            case entityRef_Type:
                if (map.stringFields.count(fieldName))
                {
                    *reinterpret_cast<std::string *>(var.data) = map.stringFields[fieldName];
                    if (entityPtrFields.count(fieldName))
                        entityPtrFields[fieldName].guidStorage = map.stringFields[fieldName];
                }
                break;

            case compRef_Type:
                if (map.stringFields.count(fieldName))
                {
                    *reinterpret_cast<std::string *>(var.data) = map.stringFields[fieldName];
                    if (componentPtrFields.count(fieldName))
                        componentPtrFields[fieldName].guidStorage = map.stringFields[fieldName];
                }
                break;
            }
        }
    }

    void ResolvePointers() override;

    std::vector<SerializableVariable> *GetSerializedFields() override
    {
        return &serializables;
    }

    virtual void OnFieldChanged(const std::string &fieldName) {}

    // Call this from inspector after modifying a field
    void NotifyFieldChanged(const std::string &fieldName)
    {
        OnFieldChanged(fieldName);
    }

protected:
    std::vector<SerializableVariable> serializables;
    std::map<std::string, std::string> compRefStrings;
    bool fieldsRegistered = false;
    std::map<std::string, EntityPtrField> entityPtrFields;
    std::map<std::string, ComponentPtrField> componentPtrFields;

    void *GetFieldData(const std::string &name)
    {
        for (auto &var : serializables)
            if (std::string(var.name) == name)
                return var.data;
        return nullptr;
    }
};