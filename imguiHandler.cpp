#include "imguiHandler.h"

ImguiHandler* ImguiHandler::s_instance = nullptr;


void ImguiHandler::Update(sf::Time rest)
{
	ImGui::SFML::Update(Engine::get().GetWindow(), rest);
	ImGui::Begin("!");
	if(ImGui::Button("Pause"))
	{
		Engine::get().isEngine = true;
	}
	if(ImGui::Button("Play"))
	{
		Engine::get().isEngine = false;
	}


	ImGui::End();
	if (!Engine::get().isEngine)
		return;
	ImGui::Begin("Entities");
	if(ImGui::Button("+"))
	{
		Engine::get().Spawn(new Entity("New Entity " + Engine::get().GetTotalEntities()));
	}
	Engine::get().GetManager()->DisplayEntities();
	ImGui::End();


	ImGui::Begin("Inspector");
	Engine::get().GetManager()->DisplayComponents();
	ImGui::End();
}
