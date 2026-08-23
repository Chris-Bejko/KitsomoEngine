#include "UIntField.h"
#include "SerializableScript.h"
#include "imgui.h"

void UIntField::Draw(const FieldDrawContext &context)
{
    ImGui::Text("%s", name);
    unsigned int temp = *value;
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputScalar(("##" + std::string(name) + "##" + context.fieldId).c_str(), ImGuiDataType_U32, &temp))
    {
        *value = temp;
        context.script->NotifyFieldChanged(name);
    }
}