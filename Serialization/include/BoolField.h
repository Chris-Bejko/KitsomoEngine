#pragma once
#include "SerializedField.h"
#include "imgui.h"

class BoolField : public SerializedField
{
    bool *value;

public:
    BoolField(const char *name, bool *val) : SerializedField(name), value(val) {}
    void Draw(const FieldDrawContext &context) override;
    std::string Serialize() override
    {
        return std::to_string(*value);
    }
    void Deserialize(const std::string &input) override
    {
        try
        {
            *value = std::stoi(input);
        }
        catch (...)
        {
            *value = 0;
        }
    }
};