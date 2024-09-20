#include "imguiHandler.h"

ImguiHandler* ImguiHandler::s_instance = nullptr;

void ImguiHandler::DisplayComponents(ComponentList list)
{
}

void ImguiHandler::DisplayEntities(std::vector<std::unique_ptr<Entity>> entities)
{
}

void ImguiHandler::Update(sf::Time rest)
{
	ImGui::SFML::Update(Engine::get().GetWindow(), rest);

	ImGui::Begin("Entities");
	ImGui::Button("+");
	Engine::get().GetManager()->DisplayEntities();
	ImGui::End();


	ImGui::Begin("Inspector");
	Engine::get().GetManager()->DisplayComponents();
	ImGui::End();
}
