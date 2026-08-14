#include "Engine.h"
#include "SceneManager.h"
#include "EntitiesDialog.h"
#include "ColorPalletes.h"
#include "DialogManager.h"
REGISTER_DIALOG(EntitiesDialog);

EntitiesDialog::EntitiesDialog() {};
void EntitiesDialog::Open()
{
}
void EntitiesDialog::Close()
{
}

void EntitiesDialog::Draw()
{
	ImGui::Begin("Entities");

	// Add entity button
	ImGui::PushStyleColor(ImGuiCol_Button, COLOR_SUCCESS);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.75f, 0.45f, 1.0f));
	if (ImGui::Button("+ New Entity", ImVec2(-1, 28)))
		Engine::get().Spawn(new Entity(
			Engine::get().GetManager()->GetUniqueName("New Entity")));
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

	Engine::get().GetManager()->DisplayEntities();
	ImGui::End();
}
