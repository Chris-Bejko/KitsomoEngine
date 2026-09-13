#include "StringField.h"
#include "SerializableScript.h"
#include "imgui.h"

void StringField::Draw(const FieldDrawContext &context)
{
    ImGui::Text("%s", name);
    ImGui::SetNextItemWidth(-1);
    char buffer[256];
    strncpy(buffer, value->c_str(), sizeof(buffer));
    if (ImGui::InputText(("##" + std::string(name) + "##" + context.fieldId).c_str(), buffer, sizeof(buffer)))
    {
        *value = std::string(buffer);
        context.script->NotifyFieldChanged(name);
    }
}