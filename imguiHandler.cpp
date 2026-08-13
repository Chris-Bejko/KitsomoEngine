#include "imguiHandler.h"
#include "Logger.h"
#include "SceneManager.h"
#include "Commands/DeleteEntityCommand.h"
#include "Commands/CopyEntityCommand.h"
#include "Commands/PasteEntityCommand.h"
#include "Commands/DuplicateEntityCommand.h"
#include "GizmoSystem.h"
#include "HotReloading/ScriptCompiler.h"
#include <filesystem>
#include <algorithm>

ImguiHandler *ImguiHandler::s_instance = nullptr;

// Color palette
static const ImVec4 COLOR_ACCENT = ImVec4(0.25f, 0.52f, 0.95f, 1.0f);
static const ImVec4 COLOR_SUCCESS = ImVec4(0.18f, 0.65f, 0.35f, 1.0f);
static const ImVec4 COLOR_DANGER = ImVec4(0.75f, 0.18f, 0.18f, 1.0f);
static const ImVec4 COLOR_WARNING = ImVec4(0.85f, 0.60f, 0.10f, 1.0f);
static const ImVec4 COLOR_PANEL_BG = ImVec4(0.11f, 0.11f, 0.13f, 1.0f);
static const ImVec4 COLOR_HEADER = ImVec4(0.18f, 0.18f, 0.22f, 1.0f);
static const ImVec4 COLOR_TEXT_DIM = ImVec4(0.55f, 0.55f, 0.60f, 1.0f);

void ImguiHandler::ApplyEditorStyle()
{
	ImGuiStyle &style = ImGui::GetStyle();
	style.WindowRounding = 6.0f;
	style.FrameRounding = 4.0f;
	style.PopupRounding = 4.0f;
	style.ScrollbarRounding = 4.0f;
	style.GrabRounding = 4.0f;
	style.TabRounding = 4.0f;
	style.WindowPadding = ImVec2(12, 12);
	style.FramePadding = ImVec2(8, 4);
	style.ItemSpacing = ImVec2(8, 6);
	style.ItemInnerSpacing = ImVec2(6, 4);
	style.ScrollbarSize = 10.0f;
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.SeparatorTextBorderSize = 1.0f;

	ImVec4 *colors = style.Colors;
	colors[ImGuiCol_WindowBg] = COLOR_PANEL_BG;
	colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.13f, 0.16f, 1.0f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.13f, 0.16f, 1.0f);
	colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.30f, 1.0f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.20f, 1.0f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.28f, 1.0f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.32f, 1.0f);
	colors[ImGuiCol_TitleBg] = COLOR_HEADER;
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.26f, 1.0f);
	colors[ImGuiCol_Header] = ImVec4(0.22f, 0.40f, 0.70f, 0.4f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.45f, 0.80f, 0.5f);
	colors[ImGuiCol_HeaderActive] = COLOR_ACCENT;
	colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.26f, 1.0f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.50f, 0.90f, 0.8f);
	colors[ImGuiCol_ButtonActive] = COLOR_ACCENT;
	colors[ImGuiCol_CheckMark] = COLOR_ACCENT;
	colors[ImGuiCol_SliderGrab] = COLOR_ACCENT;
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.65f, 1.0f, 1.0f);
	colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.20f, 1.0f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.45f, 0.80f, 0.8f);
	colors[ImGuiCol_TabActive] = COLOR_ACCENT;
	colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.32f, 1.0f);
	colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.92f, 1.0f);
	colors[ImGuiCol_TextDisabled] = COLOR_TEXT_DIM;
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.0f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.32f, 1.0f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.42f, 1.0f);
}

void ImguiHandler::Update(sf::Time rest)
{
	if (Engine::get().IsLoading())
		return;
	auto& engine = Engine::get();
	auto& window = engine.GetWindow();

	std::cout << "Engine: " << &engine << '\n';
	std::cout << "Window: " << &window << '\n';

	auto handle = window.getSystemHandle();

	std::cout << "Handle: " << handle << '\n';

	ImGui::SFML::Update(window, rest);
	DrawToolbar();
	DrawStatusWindow();
	// DrawConsole();
	// DrawInspector();
	DrawEntities();
	DrawProjectExplorer();
	DrawProjectLoadWindow();

	if (Engine::get().GetCurrentState() == EngineState::Running && SceneManager::get().HasProjectRoot())
		DrawScenePanel();
	if (savePressed)
		DrawSaveDialog();
	if (loadPressed)
		DrawLoadDialog();
	if (showDeleteDialog)
		DrawDeleteConfirmDialog();

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) &&
		sf::Keyboard::isKeyPressed(sf::Keyboard::S) && !savePressed)
	{
		str = "saveFile.txt";
		str.resize(255, '\0');
		savePressed = true;
	}

	// Handle entity keyboard shortcuts only in editor mode
	if (Engine::get().GetCurrentState() == EngineState::Running)
	{
		HandleEntityKeyboardShortcuts();
	}
}


void ImguiHandler::DrawScriptStatus()
{
    if (Engine::get().pendingRecompile)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
                           "● Recompiling...");
    }
    else if (ScriptCompiler::lastCompileFailed)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
                           "● Compile Error");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", ScriptCompiler::lastError.c_str());
    }
    else
    {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
                           "● Scripts OK");
    }
}

void ImguiHandler::DrawToolbar()
{
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::Begin("##toolbar", nullptr,
				 ImGuiWindowFlags_NoTitleBar |
					 ImGuiWindowFlags_NoResize |
					 ImGuiWindowFlags_NoScrollbar);

	auto stateButton = [](const char *label, ImVec4 color, bool &pressed)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, color);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x + 0.1f, color.y + 0.1f, color.z + 0.1f, 1.0f));
		pressed = ImGui::Button(label, ImVec2(70, 28));
		ImGui::PopStyleColor(2);
		return pressed;
	};

	bool pressed = false;
	if (!SceneManager::get().HasProjectRoot())
	{
		if (stateButton(" Open", COLOR_ACCENT, pressed))
			showOpenProjectDialog = true;
		ImGui::SameLine();
		if (stateButton("  New", COLOR_SUCCESS, pressed))
			showNewProjectDialog = true;
		ImGui::SameLine();
		ImGui::TextColored(COLOR_WARNING, "No project loaded");
		ImGui::End();
		return;
	}

	switch (Engine::get().GetCurrentState())
	{
	case EngineState::Running:
		if (stateButton("  Play", COLOR_SUCCESS, pressed))
			OnPlay();
		ImGui::SameLine();
		if (stateButton("  Save", COLOR_ACCENT, pressed))
			OnSave();
		ImGui::SameLine();
		if (stateButton("  Load", COLOR_ACCENT, pressed))
			OnLoad();
		break;

	case EngineState::PlayMode:
		if (stateButton(" Pause", COLOR_WARNING, pressed))
			OnPause();
		ImGui::SameLine();
		if (stateButton("  Save", COLOR_ACCENT, pressed))
			OnSave();
		ImGui::SameLine();
		if (stateButton("  Load", COLOR_ACCENT, pressed))
			OnLoad();
		break;

	case EngineState::Paused:
		if (stateButton("  Play", COLOR_SUCCESS, pressed))
			OnPlay();
		break;
	}

	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, COLOR_DANGER);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.28f, 0.28f, 1.0f));
	if (ImGui::Button(" Reset", ImVec2(70, 28)))
		OnReset();
	ImGui::PopStyleColor(2);

	ImGui::SameLine();
	ImGui::Separator();
	ImGui::SameLine();

	auto gizmoButton = [](const char *label, GizmoMode btnMode)
	{
		bool active = GizmoSystem::get().GetMode() == btnMode;
		if (active)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.52f, 0.95f, 1.0f));
		if (ImGui::Button(label, ImVec2(40, 28)))
			GizmoSystem::get().SetMode(btnMode);
		if (active)
			ImGui::PopStyleColor();
	};

	gizmoButton("W", GizmoMode::Move);
	ImGui::SameLine();
	gizmoButton("E", GizmoMode::Rotate);
	ImGui::SameLine();
	gizmoButton("R", GizmoMode::Scale);
	ImGui::SameLine();

	// Snap toggle
	bool snap = GizmoSystem::get().snapEnabled;
	if (snap)
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.60f, 0.10f, 1.0f));
	if (ImGui::Button("Snap", ImVec2(50, 28)))
		GizmoSystem::get().snapEnabled = !GizmoSystem::get().snapEnabled;
	if (snap)
		ImGui::PopStyleColor();

	ImGui::SameLine();
	DrawScriptStatus();
	ImGui::SameLine();
	if (ImGui::Button("Reload Scripts", ImVec2(110, 28)))
	{
		Engine::get().RequestScriptRecompile();
	}
	ImGui::End();
}

void ImguiHandler::DrawEntities()
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

void ImguiHandler::DrawInspector()
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

	Engine::get().GetManager()->DisplayComponentsOf(displayEntity);

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
void ImguiHandler::DrawSaveDialog()
{
	if (Engine::get().GetCurrentState() != EngineState::Running)
		return;

	ImGui::SetNextWindowSize(ImVec2(380, 0), ImGuiCond_Always);
	ImGui::Begin("Save Project", &savePressed);

	ImGui::TextColored(COLOR_TEXT_DIM, "Filename");
	ImGui::SetNextItemWidth(-1);
	ImGui::InputText("##savefile", &str[0], 255);
	ImGui::Spacing();

	ImGui::PushStyleColor(ImGuiCol_Button, COLOR_SUCCESS);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.75f, 0.45f, 1.0f));
	if (ImGui::Button("Save", ImVec2(-1, 28)))
	{
		Engine::get().Save(str.c_str());
		savePressed = false;
		LOG_INFO("Saved to: ", str.c_str());
	}
	ImGui::PopStyleColor(2);

	ImGui::PushStyleColor(ImGuiCol_Button, COLOR_DANGER);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.28f, 0.28f, 1.0f));
	if (ImGui::Button("Cancel", ImVec2(-1, 28)))
		savePressed = false;
	ImGui::PopStyleColor(2);

	ImGui::End();
}

void ImguiHandler::DrawLoadDialog()
{
	if (Engine::get().GetCurrentState() != EngineState::Running)
		return;

	ImGui::SetNextWindowSize(ImVec2(380, 0), ImGuiCond_Always);
	ImGui::Begin("Load Project", &loadPressed);

	ImGui::TextColored(COLOR_TEXT_DIM, "Filename");
	ImGui::SetNextItemWidth(-1);
	ImGui::InputText("##loadfile", &str[0], 255);
	ImGui::Spacing();

	if (loadError)
		ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "File not found!");

	ImGui::PushStyleColor(ImGuiCol_Button, COLOR_ACCENT);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.62f, 1.0f, 1.0f));
	if (ImGui::Button("Load", ImVec2(-1, 28)))
	{
		if (Engine::get().Load(str.c_str()))
		{
			loadPressed = false;
			loadError = false;
			LOG_INFO("Loaded: ", str.c_str());
		}
		else
		{
			loadError = true;
			LOG_WARNING("File not found: ", str.c_str());
		}
	}
	ImGui::PopStyleColor(2);

	ImGui::PushStyleColor(ImGuiCol_Button, COLOR_DANGER);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.28f, 0.28f, 1.0f));
	if (ImGui::Button("Cancel", ImVec2(-1, 28)))
	{
		loadPressed = false;
		loadError = false;
	}
	ImGui::PopStyleColor(2);

	ImGui::End();
}

void ImguiHandler::OnPlay()
{
	savePressed = false;
	loadPressed = false;
	Engine::get().SetEngineState(EngineState::PlayMode);
}
void ImguiHandler::OnPause() { Engine::get().SetEngineState(EngineState::Paused); }
void ImguiHandler::OnReset()
{
	savePressed = false;
	loadPressed = false;
	Engine::get().SetEngineState(EngineState::Running);
}
void ImguiHandler::OnSave()
{
	if (!SceneManager::get().HasProjectRoot())
	{
		Notify("Load a project before saving", COLOR_WARNING);
		return;
	}

	str = "saveFile.txt";
	str.resize(255, '\0');
	savePressed = true;
}
void ImguiHandler::OnLoad()
{
	if (!SceneManager::get().HasProjectRoot())
	{
		showOpenProjectDialog = true;
		Notify("Select a project before loading scenes", COLOR_WARNING);
		return;
	}

	str = "saveFile.txt";
	str.resize(255, '\0');
	loadPressed = true;
}
void ImguiHandler::ClearInspector() { Engine::get().ClearInpsector(); }

void ImguiHandler::Notify(const std::string &message, ImVec4 color, float lifetime)
{
	notifications.push_back({message, color, lifetime, lifetime});
	if (notifications.size() > 5)
		notifications.pop_front();
}

void ImguiHandler::DrawStatusWindow()
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin("##status", nullptr,
				 ImGuiWindowFlags_NoTitleBar |
					 ImGuiWindowFlags_NoResize |
					 ImGuiWindowFlags_NoScrollbar |
					 ImGuiWindowFlags_AlwaysAutoResize);

	float dt = Engine::get().GetDt(); 
	for (auto it = notifications.begin(); it != notifications.end();)
	{
		it->lifetime -= dt;
		float alpha = std::min(1.0f, it->lifetime / 0.5f); // fade out last 0.5s
		ImVec4 color = it->color;
		color.w = alpha;

		// Progress bar for lifetime
		float progress = it->lifetime / it->maxLifetime;
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(color.x, color.y, color.z, 0.4f));
		ImGui::ProgressBar(progress, ImVec2(-1, 3), "");
		ImGui::PopStyleColor();

		ImGui::TextColored(color, it->message.c_str());
		ImGui::Spacing();

		if (it->lifetime <= 0.f)
			it = notifications.erase(it);
		else
			++it;
	}
	ImGui::End();
};

// void ImguiHandler::AddConsoleLog(const std::string &message, ImVec4 color)
// {
// 	// consoleLogs.push_back({message, color});
// 	// if (consoleLogs.size() > 200)
// 	// 	consoleLogs.pop_front();
// }
// ImVec4 GetValueColor(LogLevel level)
// {
// 	switch (level)
// 	{
// 	case LogLevel::Info:
// 		return ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
// 	case LogLevel::Warning:
// 		return ImVec4(0.95f, 0.78f, 0.2f, 1.0f);
// 	case LogLevel::Error:
// 		return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
// 	case LogLevel::Debug:
// 		return ImVec4(0.4f, 0.85f, 1.0f, 1.0f);
// 	default:
// 		return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
// 	}
// }
// void ImguiHandler::DrawConsole()
// {
// 	ImGui::Begin("Console");

// 	// Clear button
// 	ImGui::PushStyleColor(ImGuiCol_Button, COLOR_DANGER);
// 	if (ImGui::Button("Clear", ImVec2(70, 24)))
// 		consoleLogs.clear();
// 	ImGui::PopStyleColor();
// 	ImGui::SameLine();
// 	ImGui::TextColored(COLOR_TEXT_DIM, "%zu entries", consoleLogs.size());

// 	ImGui::Separator();
// 	ImGui::Spacing();

// 	ImGui::BeginChild("##consolescroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
// 	for (auto &[msg, color] : consoleLogs)
// 		ImGui::TextColored(color, msg.c_str());

// 	// Auto scroll to bottom
// 	if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
// 		ImGui::SetScrollHereY(1.0f);

// 	ImGui::EndChild();
// 	ImGui::End();
// }

void ImguiHandler::DrawProjectExplorer()
{
	ImGui::Begin("Project Explorer");

	if (ImGui::Button("New Project", ImVec2(100, 26)))
		showNewProjectDialog = true;
	ImGui::SameLine();
	if (ImGui::Button(SceneManager::get().HasProjectRoot() ? "Refresh" : "Open Project", ImVec2(90, 26)))
	{
		if (SceneManager::get().HasProjectRoot())
			projectExplorerDirectory = std::filesystem::path();
		else
			showOpenProjectDialog = true;
	}

	if (!SceneManager::get().HasProjectRoot())
	{
		ImGui::Separator();
		ImGui::TextColored(COLOR_TEXT_DIM, "PROJECT ROOT");
		ImGui::TextColored(COLOR_WARNING, "No project selected");
		ImGui::Spacing();
		ImGui::TextWrapped("Open or create a project to browse assets and scenes.");

		if (showNewProjectDialog)
		{
			ImGui::OpenPopup("Create New Project");
			showNewProjectDialog = false;
		}

		if (ImGui::BeginPopupModal("Create New Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Choose a project name:");
			char buffer[128] = {0};
			std::strncpy(buffer, newProjectNameBuffer.c_str(), sizeof(buffer) - 1);
			if (ImGui::InputText("##newProjectName", buffer, sizeof(buffer)))
				newProjectNameBuffer = buffer;
			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				if (SceneManager::get().CreateNewProject(newProjectNameBuffer))
				{
					projectExplorerDirectory = std::filesystem::path();
					newProjectNameBuffer = "MyProject";
					showOpenProjectDialog = false;
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		ImGui::End();
		return;
	}

	ImGui::Separator();
	ImGui::TextColored(COLOR_TEXT_DIM, "PROJECT ROOT");
	ImGui::TextWrapped("%s", SceneManager::get().GetProjectRoot().c_str());
	ImGui::Spacing();

	if (showNewProjectDialog)
	{
		ImGui::OpenPopup("Create New Project");
		showNewProjectDialog = false;
	}

	if (ImGui::BeginPopupModal("Create New Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Choose a project name:");
		char buffer[128] = {0};
		std::strncpy(buffer, newProjectNameBuffer.c_str(), sizeof(buffer) - 1);
		if (ImGui::InputText("##newProjectName", buffer, sizeof(buffer)))
			newProjectNameBuffer = buffer;
		if (ImGui::Button("Create", ImVec2(120, 0)))
		{
			if (SceneManager::get().CreateNewProject(newProjectNameBuffer))
			{
				projectExplorerDirectory = std::filesystem::path();
				newProjectNameBuffer = "MyProject";
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	ImGui::Spacing();
	ImGui::TextColored(COLOR_TEXT_DIM, "ASSETS");
	ImGui::Separator();

	std::filesystem::path baseDir = projectExplorerDirectory.empty()
		? SceneManager::get().GetProjectRootPath()
		: projectExplorerDirectory;

	if (!std::filesystem::exists(baseDir))
		baseDir = SceneManager::get().GetProjectRootPath();

	if (!projectExplorerDirectory.empty())
	{
		if (ImGui::Button("..", ImVec2(-1, 24)))
			projectExplorerDirectory = baseDir.parent_path();
	}

	std::vector<std::filesystem::directory_entry> entries;
	for (const auto &entry : std::filesystem::directory_iterator(baseDir))
		entries.push_back(entry);
	std::sort(entries.begin(), entries.end(), [](const auto &a, const auto &b)
	{
		return a.path().filename().string() < b.path().filename().string();
	});

	for (const auto &entry : entries)
	{
		const auto path = entry.path();
		const std::string name = path.filename().string();
		const bool isDirectory = std::filesystem::is_directory(path);
		const bool shouldIgnore = name == "Generated" || name == "build";
		if (isDirectory && ! shouldIgnore)
		{
			if (ImGui::Button(("[DIR] " + name).c_str(), ImVec2(-1, 24)))
				projectExplorerDirectory = path;
		}
		else
		{
			const std::string ext = path.extension().string();
			if (ext == ".scene")
			{
				if (ImGui::Button(("[SCENE] " + name).c_str(), ImVec2(-1, 24)))
					SceneManager::get().LoadScene(path.stem().string(), SceneLoadMode::Replace);
			}
			else if (ext == ".prefab")
			{
				ImGui::Button(("[PREFAB] " + name).c_str(), ImVec2(-1, 24));
			}
			else if (!isDirectory)
			{
				ImGui::Button(name.c_str(), ImVec2(-1, 24));
			}
		}
	}

	ImGui::Spacing();
	ImGui::TextColored(COLOR_TEXT_DIM, "ACTIONS");
	ImGui::Separator();
	if (ImGui::Button("New Scene", ImVec2(-1, 28)))
	{
		std::string sceneName = "NewScene";
		std::filesystem::path scenePath = baseDir / (sceneName + ".scene");
		int suffix = 1;
		while (std::filesystem::exists(scenePath))
		{
			scenePath = baseDir / (sceneName + std::to_string(suffix++) + ".scene");
		}
		SceneManager::get().SaveSceneAs(scenePath.stem().string());
		projectExplorerDirectory = baseDir;
	}

	ImGui::End();
}

void ImguiHandler::DrawScenePanel()
{
	if (!SceneManager::get().HasProjectRoot())
		return;

	ImGui::Begin("Scenes");

	// Current scene
	ImGui::TextColored(COLOR_TEXT_DIM, "CURRENT SCENE");
	std::string current = SceneManager::get().GetCurrentScene();
	if (current.empty())
		ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Unsaved scene");
	else
		ImGui::TextColored(COLOR_ACCENT, current.c_str());

	ImGui::Separator();
	ImGui::Spacing();

	// Save buttons
	ImGui::PushStyleColor(ImGuiCol_Button, COLOR_SUCCESS);
	if (ImGui::Button("Save Scene", ImVec2(-1, 28)))
	{
		if (current.empty())
			saveScenePressed = true;
		else
			SceneManager::get().SaveCurrentScene();
	}
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_Button, COLOR_ACCENT);
	if (ImGui::Button("Save Scene As...", ImVec2(-1, 28)))
		saveScenePressed = true;
	ImGui::PopStyleColor();

	if (saveScenePressed)
	{
		ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Always);
		ImGui::Begin("Save Scene As", &saveScenePressed);
		ImGui::SetNextItemWidth(-1);
		ImGui::InputText("##scenename", &sceneNameBuffer[0], 128);
		ImGui::PushStyleColor(ImGuiCol_Button, COLOR_SUCCESS);
		if (ImGui::Button("Save", ImVec2(-1, 28)))
		{
			SceneManager::get().SaveSceneAs(std::string(sceneNameBuffer.c_str()));
			saveScenePressed = false;
		}
		ImGui::PopStyleColor();
		ImGui::End();
	}

	ImGui::Spacing();
	ImGui::TextColored(COLOR_TEXT_DIM, "AVAILABLE SCENES");
	ImGui::Separator();
	ImGui::Spacing();

	// List available scenes
	auto scenes = SceneManager::get().GetAvailableScenes();
	if (scenes.empty())
	{
		ImGui::TextColored(COLOR_TEXT_DIM, "No scenes found");
	}
	else
	{
		for (auto &scene : scenes)
		{
			bool isLoaded = scene == current;

			if (isLoaded)
				ImGui::PushStyleColor(ImGuiCol_Button, COLOR_ACCENT);
			else
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.22f, 0.35f, 1.0f));

			std::string label = (isLoaded ? "* " : "  ") + scene + "##scene";
			if (ImGui::Button(label.c_str(), ImVec2(-1, 24)))
			{
				// Show load options
				selectedScene = scene;
				showLoadOptions = true;
			}
			ImGui::PopStyleColor();
		}
	}

	// Load options popup
	if (showLoadOptions && !selectedScene.empty())
	{
		ImGui::SetNextWindowSize(ImVec2(250, 0), ImGuiCond_Always);
		ImGui::Begin(("Load: " + selectedScene).c_str(), &showLoadOptions);

		ImGui::PushStyleColor(ImGuiCol_Button, COLOR_DANGER);
		if (ImGui::Button("Replace (clear current)", ImVec2(-1, 28)))
		{
			SceneManager::get().LoadScene(selectedScene, SceneLoadMode::Replace);
			showLoadOptions = false;
		}
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Button, COLOR_ACCENT);
		if (ImGui::Button("Additive (keep current)", ImVec2(-1, 28)))
		{
			SceneManager::get().LoadScene(selectedScene, SceneLoadMode::Additive);
			showLoadOptions = false;
		}
		ImGui::PopStyleColor();

		ImGui::End();
	}

	ImGui::End();
}

void ImguiHandler::DrawProjectLoadWindow()
{
	if (!showOpenProjectDialog && SceneManager::get().HasProjectRoot())
		return;

	ImGui::SetNextWindowSize(ImVec2(520, 0), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Open Project", &showOpenProjectDialog))
	{
		ImGui::End();
		return;
	}

	ImGui::TextColored(COLOR_TEXT_DIM, "DISCOVERED PROJECTS");
	ImGui::Separator();
	auto projects = SceneManager::get().GetAvailableProjects();
	if (projects.empty())
	{
		ImGui::TextColored(COLOR_TEXT_DIM, "No projects found under Projects/");
	}
	else
	{
		for (const auto& projectPath : projects)
		{
			std::string label = projectPath.filename().string();
			if (ImGui::Button(label.c_str(), ImVec2(-1, 26)))
			{
				if (Engine::get().OpenProject(projectPath.string()))
				{
					projectExplorerDirectory.clear();
					loadProjectPathError = false;
					showOpenProjectDialog = false;
				}
			}
		}
	}

	ImGui::Spacing();
	ImGui::TextColored(COLOR_TEXT_DIM, "OPEN BY PATH");
	ImGui::Separator();
	ImGui::SetNextItemWidth(-1);
	ImGui::InputText("##projectPath", &loadProjectPathBuffer[0], loadProjectPathBuffer.size() + 1);
	if (loadProjectPathError)
		ImGui::TextColored(COLOR_DANGER, "Project folder not found or invalid");

	if (ImGui::Button("Open Path", ImVec2(-1, 28)))
	{
		std::string path = loadProjectPathBuffer.c_str();
		if (Engine::get().OpenProject(path))
		{
			projectExplorerDirectory.clear();
			loadProjectPathError = false;
			showOpenProjectDialog = false;
		}
		else
		{
			loadProjectPathError = true;
		}
	}

	if (!SceneManager::get().HasProjectRoot())
	{
		ImGui::TextColored(COLOR_WARNING, "The editor is currently in no-project mode.");
	}

	ImGui::End();
}

void ImguiHandler::HandleEntityKeyboardShortcuts()
{
	if (ImGui::GetIO().WantCaptureKeyboard)
		return; // Don't handle shortcuts if imgui wants keyboard focus

	bool ctrlDown = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);

	auto selected = Engine::get().GetManager()->GetSelectedEntity();

	// Ctrl+C: Copy - only on key press transition
	bool currentCtrlC = ctrlDown && sf::Keyboard::isKeyPressed(sf::Keyboard::C);
	if (currentCtrlC && !prevCtrlC && selected != nullptr)
	{
		OnCopyEntity();
		Notify("Entity copied to clipboard", COLOR_SUCCESS);
	}
	prevCtrlC = currentCtrlC;

	// Ctrl+V: Paste - only on key press transition
	bool currentCtrlV = ctrlDown && sf::Keyboard::isKeyPressed(sf::Keyboard::V);
	if (currentCtrlV && !prevCtrlV && EntityClipboard::get().HasContent())
	{
		OnPasteEntity();
		Notify("Entity pasted", COLOR_SUCCESS);
	}
	prevCtrlV = currentCtrlV;

	// Ctrl+D: Duplicate - only on key press transition
	bool currentCtrlD = ctrlDown && sf::Keyboard::isKeyPressed(sf::Keyboard::D);
	if (currentCtrlD && !prevCtrlD && selected != nullptr)
	{
		OnDuplicateEntity();
		Notify("Entity duplicated", COLOR_SUCCESS);
	}
	prevCtrlD = currentCtrlD;

	// Delete: Show delete dialog - only on key press transition
	bool currentDelete = sf::Keyboard::isKeyPressed(sf::Keyboard::Delete);
	if (currentDelete && !prevDelete && selected != nullptr)
	{
		OnDeleteEntity();
	}
	prevDelete = currentDelete;

	// Ctrl+Z: Undo - only on key press transition
	bool currentCtrlZ = ctrlDown && sf::Keyboard::isKeyPressed(sf::Keyboard::Z);
	if (currentCtrlZ && !prevCtrlZ)
	{
		OnUndo();
		Notify("Undo: " + CommandHistory::get().GetUndoDescription(), COLOR_ACCENT);
	}
	prevCtrlZ = currentCtrlZ;

	// Ctrl+Y: Redo - only on key press transition
	bool currentCtrlY = ctrlDown && sf::Keyboard::isKeyPressed(sf::Keyboard::Y);
	if (currentCtrlY && !prevCtrlY)
	{
		OnRedo();
		Notify("Redo: " + CommandHistory::get().GetRedoDescription(), COLOR_ACCENT);
	}
	prevCtrlY = currentCtrlY;
}

void ImguiHandler::DrawDeleteConfirmDialog()
{
	ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Always);
	ImGui::Begin("Delete Entity", &showDeleteDialog, ImGuiWindowFlags_AlwaysAutoResize);

	if (entityToDelete)
	{
		ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Delete entity:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), entityToDelete->GetName().c_str());
		ImGui::Text("This action cannot be undone.");
		ImGui::Spacing();

		ImGui::PushStyleColor(ImGuiCol_Button, COLOR_DANGER);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.38f, 0.38f, 1.0f));
		if (ImGui::Button("Delete", ImVec2(140, 28)))
		{
			// Create and execute the delete command
			auto deleteCmd = std::make_unique<DeleteEntityCommand>(entityToDelete);
			CommandHistory::get().Execute(std::move(deleteCmd));
			Notify("Entity deleted", COLOR_DANGER);
			showDeleteDialog = false;
			entityToDelete = nullptr;
		}
		ImGui::PopStyleColor(2);

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.22f, 0.35f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.35f, 0.55f, 1.0f));
		if (ImGui::Button("Cancel", ImVec2(140, 28)))
		{
			showDeleteDialog = false;
			entityToDelete = nullptr;
		}
		ImGui::PopStyleColor(2);
	}

	ImGui::End();
}

void ImguiHandler::OnDeleteEntity()
{
	auto selected = Engine::get().GetManager()->GetSelectedEntity();
	if (selected != nullptr)
	{
		entityToDelete = selected;
		showDeleteDialog = true;
	}
}

void ImguiHandler::OnCopyEntity()
{
	auto selected = Engine::get().GetManager()->GetSelectedEntity();
	if (selected != nullptr)
	{
		auto copyCmd = std::make_unique<CopyEntityCommand>(selected);
		CommandHistory::get().Execute(std::move(copyCmd));
	}
}

void ImguiHandler::OnPasteEntity()
{
	if (EntityClipboard::get().HasContent())
	{
		auto pasteCmd = std::make_unique<PasteEntityCommand>(10.0f, 0.0f);
		CommandHistory::get().Execute(std::move(pasteCmd));
	}
}

void ImguiHandler::OnDuplicateEntity()
{
	auto selected = Engine::get().GetManager()->GetSelectedEntity();
	if (selected != nullptr)
	{
		auto dupCmd = std::make_unique<DuplicateEntityCommand>(selected, 10.0f, 0.0f);
		CommandHistory::get().Execute(std::move(dupCmd));
	}
}

void ImguiHandler::OnUndo()
{
	if (CommandHistory::get().CanUndo())
	{
		CommandHistory::get().Undo();
	}
}

void ImguiHandler::OnRedo()
{
	if (CommandHistory::get().CanRedo())
	{
		CommandHistory::get().Redo();
	}
}