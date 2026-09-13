#include "SerializableScript.h"
#include "SerializedField.h"
#include "EntityVectorField.h"
#include "AssetRefField.h"
#include "IntField.h"
#include "FloatField.h"
#include "StringField.h"
#include "BoolField.h"
#include "UIntField.h"
#include "EntityRefField.h"

void SerializableScript::Field(const char* name, Entity*& ptr)
{
    serializedFields.push_back(std::make_unique<EntityRefField>(name, ptr));
}
// Field for vector of Entity*
void SerializableScript::Field(const char *name, std::vector<Entity *> &vec)
{
    serializedFields.push_back(std::make_unique<EntityVectorField>(name, &vec));
}

void SerializableScript::Field(const char *name, AssetReference &asset)
{
    serializedFields.push_back(std::make_unique<AssetRefField>(name, &asset));
}

// Primitive fields
// void Field(const char *name, int &val) { serializables.push_back({name, &val, int_Type}); }
void SerializableScript::Field(const char *name, int &val)
{
    serializedFields.push_back(std::make_unique<IntField>(name, &val));
}
void SerializableScript::Field(const char *name, float &val)
{
    serializedFields.push_back(std::make_unique<FloatField>(name, &val));
}
// void Field(const char *name, float &val) { serializables.push_back({name, &val, float_Type}); }
void SerializableScript::Field(const char *name, std::string &val)
{
    serializedFields.push_back(std::make_unique<StringField>(name, &val));
}
void SerializableScript::Field(const char *name, bool &val)
{
    serializedFields.push_back(std::make_unique<BoolField>(name, &val));
}
void SerializableScript::Field(const char *name, unsigned int &val)
{
    serializedFields.push_back(std::make_unique<UIntField>(name, &val));
}