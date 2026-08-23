#include "BoolField.h"
#include "SerializableScript.h"
#include "imgui.h"

void BoolField::Draw(const FieldDrawContext &context)
{
    ImGui::Text("%s", name);
    bool temp = *value;
    ImGui::SetNextItemWidth(-1);
    if (ImGui::Checkbox(("##" + std::string(name) + "##" + context.fieldId).c_str(), &temp))
    {
        *value = temp;
        context.script->NotifyFieldChanged(name);
    }
}