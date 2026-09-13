#pragma once
#include "SerializedField.h"
#include "imgui.h"


class FloatField : public SerializedField
{
    float* value;
public:
    FloatField(const char* name, float* val) : SerializedField(name), value(val) {}
    void Draw(const FieldDrawContext& context) override;
    std::string Serialize() override
    {
        return std::to_string(*value);
    }
    void Deserialize(const std::string& input) override
    {
        try
        {
            *value = std::stof(input);
        }
        catch (...)
        {
            *value = 0.0f;
        }
    }
};