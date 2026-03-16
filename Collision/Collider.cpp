#include "Collider.h"
#include "imgui.h"

void Collider::DrawEditorButton()
{
    if (editMode)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
        if (ImGui::Button("Stop Editing", ImVec2(-1, 24)))
            editMode = false;
        ImGui::PopStyleColor();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.7f, 1.0f));
        if (ImGui::Button("Edit Collider", ImVec2(-1, 24)))
            editMode = true;
        ImGui::PopStyleColor();
    }
}