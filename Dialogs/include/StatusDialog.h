#pragma once

#include "Dialog.h"
#include "StatusManager.h"

class StatusDialog : public Dialog
{
public:
    StatusDialog();

    void Draw() override;
    void Open() override;
    void Close() override;

private:
    StatusManager& statusManager;
};