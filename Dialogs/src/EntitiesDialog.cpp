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
	// Entities list
	ImGui::Spacing();
	ImGui::TextColored(COLOR_TEXT_DIM, "SCENE");
	ImGui::Separator();
	ImGui::Spacing();

	DisplayEntities();

	// Fill remaining window space so the drag drop target can cover empty area below items
	ImGui::Dummy(ImGui::GetContentRegionAvail());

	// Drop target for whole dialog / empty space to spawn prefabs or reparent to root
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("PREFAB"))
		{
			const char *pathStr = static_cast<const char *>(payload->Data);
			if (pathStr)
			{
				Entity *spawned = Engine::get().SpawnPrefab(pathStr, Vector2F(0, 0));
				if (spawned)
				{
					manager->SetSelectedEntity(spawned);
					GizmoSystem::get().SetSelectedEntity(spawned);
				}
			}
		}
		else if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY"))
		{
			Entity *dragged = *(Entity **)payload->Data;
			if (dragged)
			{
				dragged->SetParent(nullptr);
			}
		}
		ImGui::EndDragDropTarget();
	}

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

	// Drop target - reparent only or spawn prefab
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY"))
		{
			Entity *dragged = *(Entity **)payload->Data;
			if (dragged != e)
				dragged->SetParent(e);
		}
		else if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("PREFAB"))
		{
			const char *pathStr = static_cast<const char *>(payload->Data);
			if (pathStr)
			{
				Entity *spawned = Engine::get().SpawnPrefab(pathStr, Vector2F(0, 0));
				if (spawned)
				{
					spawned->SetParent(e);
					manager->SetSelectedEntity(spawned);
					GizmoSystem::get().SetSelectedEntity(spawned);
				}
			}
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
