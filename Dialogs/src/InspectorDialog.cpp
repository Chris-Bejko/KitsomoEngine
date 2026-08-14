#include "InspectorDialog.h"
#include "Entity.h"
#include "Engine.h"
#include "imgui.h"
#include "ColorPalletes.h"
#include "DialogManager.h"
REGISTER_DIALOG(InspectorDialog);

void InspectorDialog::Draw()
{
    ImGui::Begin("Inspector");

	bool isDragging = ImGui::GetDragDropPayload() != nullptr &&
					  ImGui::GetDragDropPayload()->IsDataType("ENTITY"); // changed to ENTITY

	Entity *dragHovered = Engine::get().GetManager()->GetDragHoveredEntity();

	// Show hovered entity during drag, otherwise show selected
	Entity *displayEntity = (isDragging && dragHovered != nullptr)
								? dragHovered
								: Engine::get().GetManager()->GetSelectedEntity();

	// Guard against stale pointer
	if (displayEntity != nullptr)
	{
		bool entityStillExists = false;
		for (auto &e : Engine::get().GetManager()->GetEntities())
		{
			if (e.get() == displayEntity)
			{
				entityStillExists = true;
				break;
			}
		}
		if (!entityStillExists)
		{
			Engine::get().GetManager()->SetSelectedEntity(nullptr);
			dragHovered = nullptr;
			displayEntity = nullptr;
		}
	}

	if (displayEntity == nullptr)
	{
		ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No entity selected.");
		ImGui::End();
		return;
	}
	if (displayEntity == nullptr)
	{
		ImGui::TextColored(COLOR_TEXT_DIM, "No entity selected.");
		ImGui::End();
		return;
	}

	if (isDragging && dragHovered != nullptr &&
		dragHovered != Engine::get().GetManager()->GetSelectedEntity())
	{
		ImGui::TextColored(ImVec4(0.95f, 0.78f, 0.2f, 1.0f),
						   "Drop target: %s", displayEntity->GetName().c_str());
		ImGui::Separator();
	}

	Engine::get().GetManager()->DisplayComponentsOf(displayEntity);

	if (!isDragging)
	{
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.22f, 0.35f, 1.0f));
		if (ImGui::Button("  Save as Prefab", ImVec2(-1, 28)))
			Engine::get().SavePrefab(displayEntity);
		ImGui::PopStyleColor();
	}

	ImGui::End();
}


void InspectorDialog::Open()
{
    SetName("Inspector");
}


void InspectorDialog::Close()
{
    // No specific close behavior for the inspector dialog
}