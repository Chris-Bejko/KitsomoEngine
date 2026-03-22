#include "SerializableScript.h"
#include "Engine.h"
#include "Entity.h"
#include "EntityManager.h"

void SerializableScript::Field(const char *name, Entity *&ptr)
{
    entityPtrFields[name] = {&ptr, ""};
    serializables.push_back({name, &entityPtrFields[name].guidStorage, entityRef_Type});
}

void SerializableScript::Field(const char *name, std::vector<Entity *> &vec)
{
    vectorStrings[name] = "";
    serializables.push_back({name, &vectorStrings[name], entityRef_Type});
    vectorPtrFields[name] = {
        [&vec, name, this]()
        {
            std::string result;
            for (size_t i = 0; i < vec.size(); i++)
            {
                result += vec[i] ? vec[i]->GetGUID() : "";
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
                for (auto &e : Engine::get().GetManager()->GetEntities())
                {
                    if (e->GetGUID() == token)
                    {
                        vec.push_back(e.get());
                        break;
                    }
                }
            }
        },
        entityRef_Type};
}

void SerializableScript::Serialize()
{
    for (auto &[name, entry] : vectorFields)
        entry.serialize();
    for (auto &[name, entry] : vectorPtrFields)
        entry.serialize();
    for (auto &[name, field] : entityPtrFields)
        if (*field.ptr)
            field.guidStorage = (*field.ptr)->GetGUID();
    for (auto &[name, field] : componentPtrFields)
        if (*field.ptr)
            field.guidStorage = (*field.ptr)->entity->GetGUID() + "|" + (*field.ptr)->GetGUID();
}

void SerializableScript::InitSerializedFields(ReadableSerializableVariableMap map)
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
            {
                *reinterpret_cast<std::string *>(var.data) = map.stringFields[fieldName];
                if (vectorFields.count(fieldName))
                    vectorFields[fieldName].deserialize(map.stringFields[fieldName]);
            }
            break;
        case bool_Type:
            if (map.boolFields.count(fieldName))
                *reinterpret_cast<bool *>(var.data) = map.boolFields[fieldName];
            else if (map.intFields.count(fieldName))
                *reinterpret_cast<bool *>(var.data) = map.intFields[fieldName] != 0;
            break;
        case mathVector_Type:
        {
            if (map.stringFields.count(fieldName))
            {
                std::stringstream ss(map.stringFields[fieldName]);
                std::string token;
                int i = 0;
                auto *data = reinterpret_cast<float *>(var.data);
                while (std::getline(ss, token, '|') && i < var.vectorSize)
                {
                    try
                    {
                        data[i++] = std::stof(token);
                    }
                    catch (...)
                    {
                        LOG_WARNING("Failed to parse vector: ", token.c_str());
                    }
                }
            }
            break;
        }
        case entityRef_Type:
            if (map.stringFields.count(fieldName))
            {
                *reinterpret_cast<std::string *>(var.data) = map.stringFields[fieldName];
                if (entityPtrFields.count(fieldName))
                    entityPtrFields[fieldName].guidStorage = map.stringFields[fieldName];
                // Store for vector resolution in ResolvePointers
                if (vectorPtrFields.count(fieldName))
                    vectorStrings[fieldName] = map.stringFields[fieldName];
            }
            break;
        case compRef_Type:
            if (map.stringFields.count(fieldName))
            {
                *reinterpret_cast<std::string *>(var.data) = map.stringFields[fieldName];
                if (componentPtrFields.count(fieldName))
                    componentPtrFields[fieldName].guidStorage = map.stringFields[fieldName];
                if (vectorPtrFields.count(fieldName))
                    vectorStrings[fieldName] = map.stringFields[fieldName];
            }
            break;
        }
    }
}

void SerializableScript::ResolvePointers()
{
    for (auto &[name, field] : entityPtrFields)
    {
        if (field.guidStorage.empty())
            continue;
        for (auto &e : Engine::get().GetManager()->GetEntities())
            if (e->GetGUID() == field.guidStorage)
            {
                *field.ptr = e.get();
                break;
            }
    }

    for (auto &[name, field] : componentPtrFields)
    {
        if (field.guidStorage.empty())
            continue;
        auto pipe = field.guidStorage.find('|');
        if (pipe == std::string::npos)
            continue;
        std::string entityGUID = field.guidStorage.substr(0, pipe);
        std::string compGUID = field.guidStorage.substr(pipe + 1);
        for (auto &e : Engine::get().GetManager()->GetEntities())
        {
            if (e->GetGUID() != entityGUID)
                continue;
            for (auto &comp : e->GetComponents())
                if (comp->GetGUID() == compGUID)
                {
                    *field.ptr = comp.get();
                    break;
                }
        }
    }

    for (auto &[name, entry] : vectorPtrFields)
        entry.resolve(vectorStrings[name]);
}