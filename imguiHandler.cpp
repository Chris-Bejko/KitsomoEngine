#include "imguiHandler.h"
#include "Logger.h"
#include "CommandHistory.h"
#include "Commands/CopyEntityCommand.h"
#include "Commands/PasteEntityCommand.h"
#include "Commands/DuplicateEntityCommand.h"
#include <filesystem>
#include <algorithm>
#include "StatusManager.h"
#include "EventSystem.h"
#include "ColorPalletes.h"

ImguiHandler &ImguiHandler::get()
{
	static ImguiHandler instance;
	return instance;
}

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
	auto &engine = Engine::get();
	auto &window = engine.GetWindow();

	auto handle = window.getSystemHandle();

	ImGui::SFML::Update(window, rest);

	// Handle entity keyboard shortcuts only in editor mode
	if (Engine::get().GetCurrentState() == EngineState::Running)
	{
		HandleKeyboardShortcuts();
	}
}

void ImguiHandler::HandleKeyboardShortcuts()
{
	if (ImGui::GetIO().WantCaptureKeyboard)
		return; // Don't handle shortcuts if imgui wants keyboard focus

	bool ctrlDown = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);

	auto selected = Engine::get().GetManager()->GetSelectedEntity();

	// Ctrl+C: Copy - only on key press transition
	bool currentCtrlC = ctrlDown && sf::Keyboard::isKeyPressed(sf::Keyboard::C);
	if (currentCtrlC && !prevCtrlC && selected != nullptr)
	{
		CopyEntity();
		StatusManager::get().Notify("Entity copied to clipboard", COLOR_SUCCESS);
	}
	prevCtrlC = currentCtrlC;

	// Ctrl+V: Paste - only on key press transition
	bool currentCtrlV = ctrlDown && sf::Keyboard::isKeyPressed(sf::Keyboard::V);
	if (currentCtrlV && !prevCtrlV && EntityClipboard::get().HasContent())
	{
		PasteEntity();
		StatusManager::get().Notify("Entity pasted", COLOR_SUCCESS);
	}
	prevCtrlV = currentCtrlV;

	// Ctrl+D: Duplicate - only on key press transition
	bool currentCtrlD = ctrlDown && sf::Keyboard::isKeyPressed(sf::Keyboard::D);
	if (currentCtrlD && !prevCtrlD && selected != nullptr)
	{
		DuplicateEntity();
		StatusManager::get().Notify("Entity duplicated", COLOR_SUCCESS);
	}
	prevCtrlD = currentCtrlD;

	// Delete: Show delete dialog - only on key press transition
	bool currentDelete = sf::Keyboard::isKeyPressed(sf::Keyboard::Delete);
	if (currentDelete && !prevDelete && selected != nullptr)
	{
		EventSystem::get().Fire(DeleteEntityEvent{selected});
	}
	prevDelete = currentDelete;

	// Ctrl+Z: Undo - only on key press transition
	bool currentCtrlZ = ctrlDown && sf::Keyboard::isKeyPressed(sf::Keyboard::Z);
	if (currentCtrlZ && !prevCtrlZ)
	{
		Undo();
		StatusManager::get().Notify("Undo: " + CommandHistory::get().GetUndoDescription(), COLOR_ACCENT);
	}
	prevCtrlZ = currentCtrlZ;

	// Ctrl+Y: Redo - only on key press transition
	bool currentCtrlY = ctrlDown && sf::Keyboard::isKeyPressed(sf::Keyboard::Y);
	if (currentCtrlY && !prevCtrlY)
	{
		Redo();
		StatusManager::get().Notify("Redo: " + CommandHistory::get().GetRedoDescription(), COLOR_ACCENT);
	}
	prevCtrlY = currentCtrlY;
}


void ImguiHandler::CopyEntity()
{
	auto selected = Engine::get().GetManager()->GetSelectedEntity();
	if (selected != nullptr)
	{
		auto copyCmd = std::make_unique<CopyEntityCommand>(selected);
		CommandHistory::get().Execute(std::move(copyCmd));
	}
}

void ImguiHandler::PasteEntity()
{
	if (EntityClipboard::get().HasContent())
	{
		auto pasteCmd = std::make_unique<PasteEntityCommand>(10.0f, 0.0f);
		CommandHistory::get().Execute(std::move(pasteCmd));
	}
}

void ImguiHandler::DuplicateEntity()
{
	auto selected = Engine::get().GetManager()->GetSelectedEntity();
	if (selected != nullptr)
	{
		auto dupCmd = std::make_unique<DuplicateEntityCommand>(selected, 10.0f, 0.0f);
		CommandHistory::get().Execute(std::move(dupCmd));
	}
}

void ImguiHandler::Undo()
{
	if (CommandHistory::get().CanUndo())
	{
		CommandHistory::get().Undo();
	}
}

void ImguiHandler::Redo()
{
	if (CommandHistory::get().CanRedo())
	{
		CommandHistory::get().Redo();
	}
}