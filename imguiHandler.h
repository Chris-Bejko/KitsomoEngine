#pragma once
#include "imgui.h"
#include "imgui-sfml.h"
#include "Engine.h"
#include <deque>

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

    void OnPlay();
    void OnPause();
    void OnReset();
    void OnSave();
    void OnLoad();

    void DrawStatusWindow();
    void DrawConsole();
    std::deque<Notification> notifications;

    std::deque<std::pair<std::string, ImVec4>> consoleLogs;
    bool loadError = false;
};