#pragma once
#include "base/Dialog.h"

class Entity;
class InspectorDialog : public Dialog
{
public:
    InspectorDialog() = default;
    virtual ~InspectorDialog() = default;

    void Draw() override;
    void Open() override;
    void Close() override;
private:
    bool addingNewComp = false;
    bool deletePressed = false;
    void DisplayAvailableComponents(Entity* displayEntity);
};