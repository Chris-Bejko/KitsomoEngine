#pragma once
#include "imgui.h"
#include "imgui-sfml.h"
#include "Engine.h"
#include <deque>
#include "Commands/CommandHistory.h"
#include "Commands/EntityClipboard.h"

struct Notification
{
    std::string message;
    ImVec4 color;
    float lifetime;
    float maxLifetime;
};

enum class EditorPanel
{
    Toolbar,
    Entities,
    Inspector,
    SaveDialog,
    LoadDialog
};

class ImguiHandler
{
public:
    static ImguiHandler &get()
    {
        static ImguiHandler instance;
        return instance;
    }

    void Update(sf::Time rest);
    void ClearInspector();
    static void ApplyEditorStyle();
    void Notify(const std::string &message, ImVec4 color = ImVec4(0.9f, 0.9f, 0.9f, 1.0f), float lifetime = 3.0f);
    void AddConsoleLog(const std::string &message, ImVec4 color = ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    bool saveScenePressed = false;
    bool showLoadOptions = false;
    std::string selectedScene = "";
    std::string sceneNameBuffer = std::string(128, '\0');

private:
    ImguiHandler() = default;
    static ImguiHandler *s_instance;

    std::string str = "saveFile.txt";
    bool savePressed = false;
    bool loadPressed = false;

    void DrawToolbar();
    void DrawEntities();
    void DrawInspector();
    void DrawSaveDialog();
    void DrawLoadDialog();
    void DrawScenePanel();
    void DrawProjectExplorer();
    void DrawProjectLoadWindow();
    void DrawScriptStatus();
    void OpenExportWindow();
    void DrawExportWindow();
    
    void OnPlay();
    void OnPause();
    void OnReset();
    void OnSave();
    void OnLoad();

    void DrawStatusWindow();
    void DrawConsole();
    void DrawDeleteConfirmDialog();
    void HandleEntityKeyboardShortcuts();
    void OnDeleteEntity();
    void OnCopyEntity();
    void OnPasteEntity();
    void OnDuplicateEntity();
    void OnUndo();
    void OnRedo();
    
    std::deque<Notification> notifications;

    std::deque<std::pair<std::string, ImVec4>> consoleLogs;
    bool loadError = false;
    
    bool showDeleteDialog = false;
    Entity* entityToDelete = nullptr;
    bool showNewProjectDialog = false;
    bool showOpenProjectDialog = true;
    std::string newProjectNameBuffer = "MyProject";
    std::string loadProjectPathBuffer = "Projects";
    bool loadProjectPathError = false;
    std::filesystem::path projectExplorerDirectory;
    
    // Keyboard state tracking for single-key-press detection
    bool prevCtrlC = false;
    bool prevCtrlV = false;
    bool prevCtrlD = false;
    bool prevDelete = false;
    bool prevCtrlZ = false;
    bool prevCtrlY = false;

    bool exportWindowOpen = false;

    char exportGameName[256] = "My Game";
    char exportExecutableName[256] = "MyGame";
    char exportOutputPath[1024] = "";

    int exportSceneIndex = 0;

    bool exportCopyAssets = true;
    bool exportCopyOpenAL = true;

    std::vector<std::string> exportScenes;
};