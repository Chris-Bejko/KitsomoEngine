#pragma once
#include "SerializedField.h"
#include "imgui.h"

class StringField : public SerializedField
{
    std::string *value;

public:
    StringField(const char *name, std::string *val) : SerializedField(name), value(val)
    {
    }
    void Draw(const FieldDrawContext &context) override;
    std::string Serialize() override
    {
        return *value;
    }
    void Deserialize(const std::string &input) override
    {
        try
        {
            *value = input;
        }
        catch (...)
        {
            *value = "";
        }
    }
};