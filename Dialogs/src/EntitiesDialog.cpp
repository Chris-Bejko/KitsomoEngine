#include "Engine.h"
#include "SceneManager.h"
#include "EntitiesDialog.h"
#include "ColorPalletes.h"
#include "DialogManager.h"
#include "Entity.h"
#include "GizmoSystem.h"
REGISTER_DIALOG(EntitiesDialog);

EntitiesDialog::EntitiesDialog() {};
void EntitiesDialog::Open()
{
	manager = Engine::get().GetManager();
}
void EntitiesDialog::Close()
{
}

void EntitiesDialog::Draw()
{
	manager = Engine::get().GetManager();
	if (!manager)
		return;
	ImGui::Begin("Entities");

	// Add entity button
	ImGui::PushStyleColor(ImGuiCol_Button, COLOR_SUCCESS);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.75f, 0.45f, 1.0f));
	if (ImGui::Button("+ New Entity", ImVec2(-1, 28)))
		Engine::get().Spawn(new Entity(
			manager->GetUniqueName("New Entity")));
	ImGui::PopStyleColor(2);

	// Prefabs section
	std::filesystem::path prefabDir = SceneManager::get().GetPrefabDirectory();
	if (std::filesystem::exists(prefabDir))
	{
		ImGui::Spacing();
		ImGui::TextColored(COLOR_TEXT_DIM, "PREFABS");
		ImGui::Separator();
		ImGui::Spacing();

		for (const auto &entry : std::filesystem::directory_iterator(prefabDir))
		{
			std::string prefabFile = entry.path().stem().string();
			std::string buttonLabel = "  " + prefabFile + "##prefab";
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.22f, 0.35f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.35f, 0.55f, 1.0f));
			if (ImGui::Button(buttonLabel.c_str(), ImVec2(-1, 24)))
				Engine::get().LoadPrefab(prefabFile);
			ImGui::PopStyleColor(2);
		}
	}

	// Entities list
	ImGui::Spacing();
	ImGui::TextColored(COLOR_TEXT_DIM, "SCENE");
	ImGui::Separator();
	ImGui::Spacing();

	DisplayEntities();
	ImGui::End();
}


void EntitiesDialog::DisplayEntities()
{
	if(!manager)
		return;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));

	if (ImGui::GetDragDropPayload() == nullptr)
		dragHoveredEntity = nullptr;

	for (auto &e : manager->GetEntities())
	{
		if (!e || e->IsPendingDestroy())
			continue;
		if (e->HasParent())
			continue;
		DisplayEntityNode(e.get());
	}

	ImGui::PopStyleVar();
}

void EntitiesDialog::DisplayEntityNode(Entity *e)
{

	if (!e || e->IsPendingDestroy())
		return;
	std::string name = e->GetName().c_str();
	bool isSelected = (manager->GetSelectedEntity() == e);
	bool hasChildren = !e->GetChildren().empty();

	if (isSelected)
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.7f, 0.6f));

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
							   ImGuiTreeNodeFlags_SpanAvailWidth;
	if (!hasChildren)
		flags |= ImGuiTreeNodeFlags_Leaf;
	if (isSelected)
		flags |= ImGuiTreeNodeFlags_Selected;

	bool opened = ImGui::TreeNodeEx(name.c_str(), flags);

	if (ImGui::IsItemClicked())
	{
		for (auto &a : manager->GetEntities())
			a->displayComponents = false;

		e->displayComponents = true;
		manager->SetSelectedEntity(e);
		GizmoSystem::get().SetSelectedEntity(e);

		float currentTime = ImGui::GetTime();
		if (lastClickedEntity == e && (currentTime - lastClickTime) < 0.3f)
		{
			Engine::get().FocusOnEntity(e);
			e->displayComponents = true;
		}
		lastClickedEntity = e;
		lastClickTime = currentTime;
	}

	// Track hover for preview
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::GetDragDropPayload() != nullptr)
		manager->SetDragHoveredEntity(e);

	if (ImGui::BeginDragDropSource())
	{
		Entity *ptr = e;
		ImGui::SetDragDropPayload("ENTITY", &ptr, sizeof(Entity *));
		ImGui::Text("%s", name.c_str());
		ImGui::EndDragDropSource();
	}

	// Drop target - reparent only
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY"))
		{
			Entity *dragged = *(Entity **)payload->Data;
			if (dragged != e)
				dragged->SetParent(e);
		}
		ImGui::EndDragDropTarget();
	}

	if (isSelected)
		ImGui::PopStyleColor();

	if (opened)
	{
		for (auto *child : e->GetChildren())
			DisplayEntityNode(child);
		ImGui::TreePop();
	}
}
