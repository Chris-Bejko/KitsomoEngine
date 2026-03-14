#pragma once
#include "imgui.h"
#include "imgui-sfml.h"
#include "Engine.h"

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
    static ImguiHandler& get()
    {
        static ImguiHandler instance;
        return instance;
    }

    void Update(sf::Time rest);
    void ClearInspector();
	static void ApplyEditorStyle();

private:
    ImguiHandler() = default;
    static ImguiHandler* s_instance;

    std::string str = "saveFile.txt";
    bool savePressed = false;
    bool loadPressed = false;

    void DrawToolbar();
    void DrawEntities();
    void DrawInspector();
    void DrawSaveDialog();
    void DrawLoadDialog();

    void OnPlay();
    void OnPause();
    void OnReset();
    void OnSave();
    void OnLoad();

	bool loadError = false;
};