#include "IntField.h"
#include "SerializableScript.h"
#include "imgui.h"

void IntField::Draw(const FieldDrawContext &context)
{
    ImGui::Text("%s", name);
    int temp = *value;
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputInt(("##" + std::string(name) + "##" + context.fieldId).c_str(), &temp))
    {
        *value = temp;
        context.script->NotifyFieldChanged(name);
    }
}