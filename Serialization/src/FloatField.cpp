#include "FloatField.h"
#include "SerializableScript.h"
#include "imgui.h"

void FloatField::Draw(const FieldDrawContext &context)
{
    ImGui::Text("%s", name);
    float temp = *value;
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputFloat(("##" + std::string(name) + "##" + context.fieldId).c_str(), &temp))
    {
        *value = temp;
        context.script->NotifyFieldChanged(name);
    }
}