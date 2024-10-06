#include "imguiHandler.h"

ImguiHandler* ImguiHandler::s_instance = nullptr;


void ImguiHandler::Update(sf::Time rest)
{
	ImGui::SFML::Update(Engine::get().GetWindow(), rest);
	ImGui::Begin("!");
	switch (Engine::get().GetCurrentState())
	{
	case EngineState::Running:
	{
		if (ImGui::Button("Play"))
		{
			savePressed = false;
			loadPressed = false;
			Engine::get().SetEngineState(EngineState::PlayMode);
		}
		if(ImGui::Button("Save"))
		{
			savePressed = true;
		}
		if (ImGui::Button("Load"))
		{
			loadPressed = true;
		}
		break;
	}
	case EngineState::PlayMode:
	{
		if (ImGui::Button("Pause"))
		{
			savePressed = false;
			Engine::get().SetEngineState(EngineState::Paused);
		}
		if (ImGui::Button("Save"))
		{
			savePressed = true;
		}
		if (ImGui::Button("Load"))
		{
			loadPressed = true;
		}
		break;
	}
	case EngineState::Paused:
	{
		if (ImGui::Button("Play"))
		{
			savePressed = false;
			Engine::get().SetEngineState(EngineState::PlayMode);
		}
		break;
	}
	}
	if (ImGui::Button("Reset"))
	{
		savePressed = false;
		Engine::get().SetEngineState(EngineState::Running);
	}


	ImGui::End();
	if (Engine::get().GetCurrentState() == EngineState::PlayMode)
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
	{
		if(!savePressed)
			str = "Enter filename (no extensions)";
			savePressed = true;
	}

	if (savePressed)
	{
		if (Engine::get().GetCurrentState() == EngineState::Running)
		{
			ImGui::Begin("Save as");
			ImGui::InputText("Save Project", &str[0], 55);
			if (ImGui::Button("Save"))
			{
				Engine::get().Save(str);
				savePressed = false;
			}
			if(ImGui::Button("Cancel"))
			{
				savePressed = false;
			}
			ImGui::End();
		}
	}
	if(!loadPressed)
		str = "Enter filename (no extensions)";

	if(loadPressed)
	{
		if(Engine::get().GetCurrentState() == EngineState::Running)
		{
			ImGui::Begin("Load File");
			ImGui::InputText("Load Project", &str[0], 255);
			if(ImGui::Button("Load"))
			{
				if (Engine::get().Load(str))
					loadPressed = false;
				else
					ImGui::Text("File does not exist!");
			}
			if(ImGui::Button("Cancel"))
			{
				loadPressed = false;
			}
			ImGui::End();
		}
	}
}


void ImguiHandler::ClearInspector()
{
	Engine::get().ClearInpsector();
}