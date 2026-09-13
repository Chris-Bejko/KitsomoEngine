#pragma once
#include <string>
class Entity;
class SerializableScript;

struct FieldDrawContext
{
    Entity *entity;
    SerializableScript *script;
    std::string fieldId;
};

class SerializedField
{
public:
    SerializedField(const char *name) : name(name) {}
    virtual ~SerializedField() = default;

    virtual void Draw(const FieldDrawContext &context) = 0;

    virtual std::string Serialize() = 0;
    virtual void Deserialize(const std::string &input) = 0;
    virtual void Resolve() {}
    const char *GetName() const { return name; }

protected:
    const char *name;
};