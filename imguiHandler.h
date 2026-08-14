#pragma once

#include <SFML/Graphics.hpp>

class ImguiHandler
{
public:
    static ImguiHandler& get();

    void Update(sf::Time dt);
    static void ApplyEditorStyle();

private:
    ImguiHandler() = default;
    ~ImguiHandler() = default;

    ImguiHandler(const ImguiHandler&) = delete;
    ImguiHandler& operator=(const ImguiHandler&) = delete;

    void HandleKeyboardShortcuts();

    void CopyEntity();
    void PasteEntity();
    void DuplicateEntity();
    void Undo();
    void Redo();

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