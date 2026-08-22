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

	bool isDragging = ImGui::GetDragDropPayload() != nullptr && ImGui::GetDragDropPayload()->IsDataType("ENTITY");
	bool isDraggingComponent = ImGui::GetDragDropPayload() != nullptr && ImGui::GetDragDropPayload()->IsDataType("SCRIPT");
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

	if (isDragging && dragHovered != nullptr && dragHovered != Engine::get().GetManager()->GetSelectedEntity())
	{
		ImGui::TextColored(ImVec4(0.95f, 0.78f, 0.2f, 1.0f), "Drop target: %s", displayEntity->GetName().c_str());
		ImGui::Separator();
	}

	Engine::get().GetManager()->DisplayComponentsOf(displayEntity);
	
	if (isDraggingComponent)
	{
		ImGui::Separator();
		// ImGui::Button("_______",  ImVec2(-1, 24));
		ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Drop component here to add it");
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("SCRIPT"))
		{
			const char *componentName = static_cast<const char *>(payload->Data);

			if (componentName && componentName[0] != '\0')
			{
				LOG_INFO("Inspector: adding component '", componentName, "' to entity '", displayEntity->GetName().c_str(), "'");
				displayEntity->AddComponentByName(componentName);
			}
		}

		ImGui::EndDragDropTarget();
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

void InspectorDialog::DisplayComponents()
{
	if (!displayComponents)
		return;

	// Delete entity button
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
	if (ImGui::Button("Delete Entity", ImVec2(-1, 0)))
		deletePressed = true;
	ImGui::PopStyleColor();

	if (deletePressed == 1)
	{
		std::string warning = "Delete Entity: " + this->GetName() + " ?";
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
				Engine::get().QueueDestroy(GetGUID());
				deletePressed = false;
			}
			ImGui::PopStyleColor();
			ImGui::EndPopup();
		}
	}

	ImGui::Separator();

	// Entity name
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
		DisplayAvailableComponents();

	ImGui::Separator();
	ValidateAddedComponents();

	// Component to remove (deferred to avoid modifying list while iterating)
	Component *toRemove = nullptr;

	// Track component type counts for duplicate handling
	std::map<std::string, int> componentTypeCount;

	for (auto &e : components)
	{
		std::string str(typeid(*e).name());
		str = std::regex_replace(str, std::regex("class "), "");
		if (e->GetSerializedFields() == nullptr)
			continue;

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

		// Fields
		for (auto it = e->GetSerializedFields()->begin(); it != e->GetSerializedFields()->end(); ++it)
		{
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), it->name);
			std::string fieldId = std::string(it->name) + "##" + uniqueId;
			switch (it->type)
			{
			case int_Type:
			{
				auto p = reinterpret_cast<int *>(it->data);
				int temp(*p);
				ImGui::InputInt(fieldId.c_str(), &temp);
				*p = temp;
				if (auto *script = dynamic_cast<SerializableScript *>(e.get()))
					script->NotifyFieldChanged(std::string(it->name));
				break;
			}
			case float_Type:
			{
				auto p = reinterpret_cast<float *>(it->data);
				float temp(*p);
				ImGui::InputFloat(fieldId.c_str(), &temp);
				*p = temp;
				if (auto *script = dynamic_cast<SerializableScript *>(e.get()))
					script->NotifyFieldChanged(std::string(it->name));
				break;
			}
			case char_Type:
			{
				auto p = reinterpret_cast<std::string *>(it->data);

				// Check if this is a vector field
				SerializableScript *script = dynamic_cast<SerializableScript *>(e.get());
				if (script && (script->IsVectorField(std::string(it->name))))
				{
					DrawVectorField(script, it->name, fieldId);
					break;
				}

				// Normal string field
				auto str = *p;
				str.resize(200, '\0');
				ImGui::InputText(fieldId.c_str(), &str[0], 200);
				*p = std::string(str.c_str());
				break;
			}
			case bool_Type:
			{
				auto p = reinterpret_cast<bool *>(it->data);
				bool b(*p);
				ImGui::Checkbox(fieldId.c_str(), &b);
				*p = b;
				if (auto *script = dynamic_cast<SerializableScript *>(e.get()))
					script->NotifyFieldChanged(std::string(it->name));
				break;
			}
			case mathVector_Type:
			{
				if (it->isIntVector)
				{
					auto *data = reinterpret_cast<int *>(it->data);
					switch (it->vectorSize)
					{
					case 2:
						ImGui::InputInt2(fieldId.c_str(), data);
						break;
					case 3:
						ImGui::InputInt3(fieldId.c_str(), data);
						break;
					case 4:
						ImGui::InputInt4(fieldId.c_str(), data);
						break;
					}
				}
				else
				{
					auto *data = reinterpret_cast<float *>(it->data);
					switch (it->vectorSize)
					{
					case 2:
						ImGui::InputFloat2(fieldId.c_str(), data);
						break;
					case 3:
						ImGui::InputFloat3(fieldId.c_str(), data);
						break;
					case 4:
						ImGui::InputFloat4(fieldId.c_str(), data);
						break;
					}
				}
				break;
			}
			case entityRef_Type:
			{
				SerializableScript *script = dynamic_cast<SerializableScript *>(e.get());
				if (script && script->IsVectorPtrField(std::string(it->name)))
				{
					DrawVectorField(script, it->name, fieldId);
					break;
				}
				auto p = reinterpret_cast<std::string *>(it->data);
				DrawEntityRefField(*p, fieldId);
				break;
			}
			case compRef_Type:
			{
				SerializableScript *script = dynamic_cast<SerializableScript *>(e.get());
				if (script && script->IsVectorPtrField(std::string(it->name)))
				{
					DrawVectorField(script, it->name, fieldId);
					break;
				}

				auto p = reinterpret_cast<std::string *>(it->data);
				DrawCompRefField(*p, it->componentTypeHint, fieldId);
				break;
			}
			case file_Type:
			{
				auto p = reinterpret_cast<std::string *>(it->data);

				// Show filename for display
				std::string displayName = p->empty() ? "None (drop asset here)" : std::filesystem::path(*p).filename().string();

				ImVec4 boxColor = p->empty() ? ImVec4(0.15f, 0.18f, 0.25f, 1.0f)
											 : ImVec4(0.1f, 0.25f, 0.1f, 1.0f);

				ImGui::PushStyleColor(ImGuiCol_Button, boxColor);
				ImGui::Button(displayName.c_str(), ImVec2(-1, 24));
				ImGui::PopStyleColor();

				// Drop target
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload *payload =
							ImGui::AcceptDragDropPayload("ASSET_PATH"))
					{
						std::string droppedPath =
							static_cast<const char *>(payload->Data);

						std::string ext = std::filesystem::path(droppedPath).extension().string();
						std::string fieldName(it->name);
						if (auto *script = dynamic_cast<SerializableScript *>(e.get()))
						{
							if (auto *asset = script->GetAssetReference(fieldName))
							{
								asset->SetPath(droppedPath);
								asset->Load();

								script->NotifyFieldChanged(fieldName);
							}
						}
					}
					ImGui::EndDragDropTarget();
				}

				// Right click to clear
				if (ImGui::BeginPopupContextItem(("##ctx" + fieldId).c_str()))
				{
					if (ImGui::MenuItem("Clear"))
					{
						if (auto *script = dynamic_cast<SerializableScript *>(e.get()))
						{
							std::string fieldName(it->name);

							if (auto *asset = script->GetAssetReference(fieldName))
							{
								asset->SetPath("");
							}

							script->NotifyFieldChanged(fieldName);
						}
					}
					ImGui::EndPopup();
				}

				break;
			}
			default:
				assert(0);
			}
		}
		e->DrawEditorButton();
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::Spacing();
	}

	// Remove component after iteration
	if (toRemove != nullptr)
		RemoveComponent(toRemove);
}