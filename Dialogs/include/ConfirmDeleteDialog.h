#pragma once

#include "Dialog.h"
#include "Entity.h"
class Entity;
class ConfirmDeleteDialog : public Dialog
{
public:
    ConfirmDeleteDialog();

    void Draw() override;
    void Open() override;
    void Close() override;
private:
    Entity* entityToDelete = nullptr;
};