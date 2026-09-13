#pragma once

#include "SerializedField.h"


class Component;
class ComponentRefField : public SerializedField
{
private:
    Component **value;
    std::string packedGUID;
    std::string typeHint;

public:
    ComponentRefField(const char *name, Component **value, const std::string &typeHint) : SerializedField(name), value(value), typeHint(typeHint)
    {
    }

    void Draw(const FieldDrawContext &context) override;
    std::string Serialize() override;
    void Deserialize(const std::string &serialized) override;
    void Resolve() override;
};