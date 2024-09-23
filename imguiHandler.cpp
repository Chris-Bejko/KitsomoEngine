#include "imguiHandler.h"

ImguiHandler* ImguiHandler::s_instance = nullptr;


void ImguiHandler::Update(sf::Time rest)
{
	ImGui::SFML::Update(Engine::get().GetWindow(), rest);
	ImGui::Begin("!");
	if (!Engine::get().isEngine)
	{
		if (ImGui::Button("Pause"))
		{
			savePressed = false;
			Engine::get().isEngine = true;
			Engine::get().SetPlaymode(true);
		}

	}else
	{
		if (ImGui::Button("Play"))
		{
			savePressed = false;
			Engine::get().isEngine = false;
			Engine::get().SetPlaymode(true);
		}
	}
	if (ImGui::Button("Reset"))
	{
		savePressed = false;
		Engine::get().Reset();
		Engine::get().SetPlaymode(false);
	}


	ImGui::End();
	if (!Engine::get().isEngine)
		return;
	ImGui::Begin("Entities");
	if (ImGui::Button("+"))
	{
		Engine::get().Spawn(new Entity("New Entity " + Engine::get().GetTotalEntities()));
	}
	Engine::get().GetManager()->DisplayEntities();
	ImGui::End();


	ImGui::Begin("Inspector");
	Engine::get().GetManager()->DisplayComponents();
	ImGui::End();

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		savePressed = true;

	if (savePressed)
	{
		if (!Engine::get().IsPlayMode())
		{
			std::string str("Enter Filename (no extenstions)");
			ImGui::Begin("Save as");
			ImGui::InputText("Save Project", &str[0], 55);
			if (ImGui::Button("Save"))
			{
				Engine::get().Save(str);
				savePressed = false;
			}
			ImGui::End();
		}
	}
}
