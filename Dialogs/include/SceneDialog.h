#pragma once

#include "Dialog.h"
#include <string>

class SceneDialog : public Dialog
{
public:
    SceneDialog();

    void Draw() override;
    void Open() override;
    void Close() override;

private:
    bool saveScenePressed = false;
    bool showLoadOptions = false;
    std::string selectedScene;
    std::string sceneNameBuffer = "NewScene";

    void DrawSaveSceneDialog();
    void DrawLoadOptions();
};