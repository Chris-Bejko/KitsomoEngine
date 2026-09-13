#include "InspectorDialog.h"
#include "Entity.h"
#include "Engine.h"
#include "imgui.h"
#include "ColorPalletes.h"
#include "DialogManager.h"
#include "Transform.h"
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

	// Engine::get().GetManager()->DisplayComponentsOf(displayEntity);
	if (displayEntity)
	{
		bool prev = displayEntity->displayComponents;
		displayEntity->displayComponents = true;
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
		if (ImGui::Button("Delete Entity", ImVec2(-1, 0)))
			deletePressed = true;
		ImGui::PopStyleColor();

		if (deletePressed)
		{
			std::string warning = "Delete Entity: " + displayEntity->GetName() + " ?";
			char *warningChar = &warning[0];
			ImGui::OpenPopup(warningChar);
			if (ImGui::BeginPopupModal(warningChar))
			{
				ImGui::Text("This action cannot be undone.");
				ImGui::Separator();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
					deletePressed = false;
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
				if (ImGui::Button("Confirm", ImVec2(120, 0)))
				{
					Engine::get().QueueDestroy(displayEntity->GetGUID());
					deletePressed = false;
				}
				ImGui::PopStyleColor();
				ImGui::EndPopup();
			}
		}

		ImGui::Separator();
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth = ImGui::CalcTextSize("Name").x;
		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		auto temp = GetName();
		temp.resize(55, '\0');
		ImGui::InputText("##name", &temp[0], 55);
		SetName(std::string(temp.c_str()));

		ImGui::Separator();

		// Add component button
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.4f, 0.1f, 1.0f));
		if (ImGui::Button("+ Add Component", ImVec2(-1, 0)))
			addingNewComp = !addingNewComp;
		ImGui::PopStyleColor();

		if (addingNewComp)
			DisplayAvailableComponents(displayEntity);

		ImGui::Separator();
		displayEntity->ValidateAddedComponents();

		// Component to remove (deferred to avoid modifying list while iterating)
		Component *toRemove = nullptr;

		// Track component type counts for duplicate handling
		std::map<std::string, int> componentTypeCount;
		displayEntity->displayComponents = prev;

		for (auto &e : displayEntity->GetComponents())
		{
			auto *script = dynamic_cast<SerializableScript *>(e.get());
			if (!script)
				continue;

			std::string str(typeid(*e).name());
			str = std::regex_replace(str, std::regex("class "), "");
			auto &fields = script->GetSerializedFields();

			if (str == "Transform")
			{
				auto *t = dynamic_cast<Transform *>(e.get());
				if (t && t->isUITransform)
					continue; // skip rendering
			}
			// Increment count for this component type
			int componentIndex = componentTypeCount[str]++;

			// Create unique ID for ImGui (append index if multiple of same type exist)
			std::string uniqueId = str;
			if (componentIndex > 0)
				uniqueId += " (" + std::to_string(componentIndex + 1) + ")";

			// Component header
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
			ImGui::BeginChild((uniqueId + "##child").c_str(), ImVec2(0, 0), true);

			// Component name centered
			auto windowWidth = ImGui::GetWindowSize().x;
			auto textWidth = ImGui::CalcTextSize(uniqueId.c_str()).x;
			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
			ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), uniqueId.c_str());

			// Remove button (skip Transform, it's required)
			if (str != "Transform" && str != "UIRect") // UIRect is also required for UI elements
			{
				ImGui::SameLine();
				ImGui::SetCursorPosX(windowWidth - 60);
				std::string removeLabel = "X##remove_" + uniqueId;
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
				if (ImGui::Button(removeLabel.c_str(), ImVec2(50, 0)))
					toRemove = e.get();
				ImGui::PopStyleColor();
			}

			ImGui::Separator();
			int uniqueFieldId = 0;
			for (auto &field : fields)
			{
				field->Draw({displayEntity, script, uniqueId + "##" + std::to_string(uniqueFieldId)});
				uniqueFieldId++;
			}
			// Fields
			e->DrawEditorButton();
			ImGui::EndChild();
			ImGui::PopStyleColor();
			ImGui::Spacing();
		}

		// Remove component after iteration
		if (toRemove != nullptr)
			displayEntity->RemoveComponent(toRemove);
	}
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


void InspectorDialog::DisplayAvailableComponents(Entity* displayEntity)
{
	if (addingNewComp)
	{
		auto &availableComponents = displayEntity->GetAvailableComponents();
		if (availableComponents.empty())
		{
			displayEntity->SaveAvailableComponents();
			availableComponents = displayEntity->GetAvailableComponents();
		}

		ImGui::BeginChild("Add Component");
		for (auto &comp : availableComponents)
		{
			if (ImGui::Button(comp.c_str()))
			{
				displayEntity->AddComponentByName(comp);
				addingNewComp = false;
			}
		}
		ImGui::EndChild();
	}
}