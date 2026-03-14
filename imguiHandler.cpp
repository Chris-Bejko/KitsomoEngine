#include "imguiHandler.h"
#include "Logger.h"

ImguiHandler *ImguiHandler::s_instance = nullptr;

void ImguiHandler::Update(sf::Time rest)
{
	ImGui::SFML::Update(Engine::get().GetWindow(), rest);

	DrawToolbar();

	if (Engine::get().GetCurrentState() != EngineState::PlayMode)
	{
		DrawEntities();
		DrawInspector();
	}

	if (savePressed)
		DrawSaveDialog();
	if (loadPressed)
		DrawLoadDialog();

	// Ctrl+S shortcut
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) &&
		sf::Keyboard::isKeyPressed(sf::Keyboard::S) && !savePressed)
	{
		str = "saveFile.txt";
		savePressed = true;
	}
}

void ImguiHandler::DrawToolbar()
{
	ImGui::Begin("Toolbar");

	switch (Engine::get().GetCurrentState())
	{
	case EngineState::Running:
		if (ImGui::Button("Play"))
			OnPlay();
		ImGui::SameLine();
		if (ImGui::Button("Save"))
			OnSave();
		ImGui::SameLine();
		if (ImGui::Button("Load"))
			OnLoad();
		break;

	case EngineState::PlayMode:
		if (ImGui::Button("Pause"))
			OnPause();
		ImGui::SameLine();
		if (ImGui::Button("Save"))
			OnSave();
		ImGui::SameLine();
		if (ImGui::Button("Load"))
			OnLoad();
		break;

	case EngineState::Paused:
		if (ImGui::Button("Play"))
			OnPlay();
		break;
	}

	ImGui::SameLine();
	if (ImGui::Button("Reset"))
		OnReset();

	ImGui::End();
}

void ImguiHandler::DrawEntities()
{
	ImGui::Begin("Entities");
	if (ImGui::Button("+"))
		Engine::get().Spawn(new Entity("New Entity " + std::to_string(Engine::get().GetTotalEntities())));
	Engine::get().GetManager()->DisplayEntities();
	ImGui::End();
}

void ImguiHandler::DrawInspector()
{
	ImGui::Begin("Inspector");
	Engine::get().GetManager()->DisplayComponents();
	ImGui::End();
}

void ImguiHandler::DrawSaveDialog()
{
	if (Engine::get().GetCurrentState() != EngineState::Running)
		return;

	ImGui::Begin("Save As");
	ImGui::InputText("Filename", &str[0], 255);
	if (ImGui::Button("Save"))
	{
		Engine::get().Save(str);
		savePressed = false;
		LOG_INFO("Saved to: ", str);
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		savePressed = false;
	ImGui::End();
}

void ImguiHandler::DrawLoadDialog()
{
	if (Engine::get().GetCurrentState() != EngineState::Running)
		return;

	ImGui::Begin("Load File");
	ImGui::InputText("Filename", &str[0], 255);
	if (ImGui::Button("Load"))
	{
		if (Engine::get().Load(str.c_str()))
		{
			loadPressed = false;
			loadError = false;
			LOG_INFO("Loaded: ", str);
		}
		else
		{
			loadError = true;
			LOG_WARNING("File not found: ", str);
		}
	}
	if (loadError)
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "File not found!");

	if (ImGui::Button("Cancel"))
	{
		loadPressed = false;
		loadError = false;
	}
	ImGui::End();
}

// Actions
void ImguiHandler::OnPlay()
{
	savePressed = false;
	loadPressed = false;
	Engine::get().SetEngineState(EngineState::PlayMode);
}

void ImguiHandler::OnPause()
{
	Engine::get().SetEngineState(EngineState::Paused);
}

void ImguiHandler::OnReset()
{
	savePressed = false;
	loadPressed = false;
	Engine::get().SetEngineState(EngineState::Running);
}

void ImguiHandler::OnSave()
{
	str = "saveFile.txt";
	savePressed = true;
}

void ImguiHandler::OnLoad()
{
	str = "saveFile.txt";
	str.resize(255, '\0'); // pre-allocate buffer
	loadPressed = true;
}

void ImguiHandler::ClearInspector()
{
	Engine::get().ClearInpsector();
}