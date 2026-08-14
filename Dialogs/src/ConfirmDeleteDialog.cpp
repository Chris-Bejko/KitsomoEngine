#include "ColorPalletes.h"
#include "CommandHistory.h"
#include "DeleteEntityCommand.h"
#include "DialogManager.h"
#include "StatusManager.h"
#include "EventSystem.h"
#include "ConfirmDeleteDialog.h"
REGISTER_DIALOG(ConfirmDeleteDialog);

void ConfirmDeleteDialog::Open()
{
}

void ConfirmDeleteDialog::Close()
{
}

ConfirmDeleteDialog::ConfirmDeleteDialog()
{
    EventSystem::get().Subscribe<DeleteEntityEvent>([this](const DeleteEntityEvent &event)
                                                    { entityToDelete = event.entity; });
}
void ConfirmDeleteDialog::Draw()
{
    if (!entityToDelete)
        return;

    ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Always);
    ImGui::Begin("Delete Entity", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Delete entity:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), entityToDelete->GetName().c_str());
    ImGui::Text("This action cannot be undone.");
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, COLOR_DANGER);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.38f, 0.38f, 1.0f));
    if (ImGui::Button("Delete", ImVec2(140, 28)))
    {
        // Create and execute the delete command
        auto deleteCmd = std::make_unique<DeleteEntityCommand>(entityToDelete);
        CommandHistory::get().Execute(std::move(deleteCmd));
        std::string message = "\"" + entityToDelete->GetName() + "\" deleted";
        StatusManager::get().Notify(message, COLOR_DANGER);
        entityToDelete = nullptr;
    }
    ImGui::PopStyleColor(2);

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.22f, 0.35f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.35f, 0.55f, 1.0f));
    if (ImGui::Button("Cancel", ImVec2(140, 28)))
    {
        entityToDelete = nullptr;
    }
    ImGui::PopStyleColor(2);

    ImGui::End();
}