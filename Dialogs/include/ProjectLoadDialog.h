#pragma once

#include "Dialog.h"
#include "EventSystem.h"
#include <string>

class ProjectLoadDialog : public Dialog
{
public:
    ProjectLoadDialog();

    void Draw() override;
    void Open() override;
    void Close() override;

private:
    bool showOpenProjectDialog = false;
    bool loadProjectPathError = false;
    std::string loadProjectPathBuffer;

    EventSystem::CallbackId openDialogSubscription = 0;
    EventSystem::CallbackId loadSuccessSubscription = 0;
    EventSystem::CallbackId loadFailedSubscription = 0;
};