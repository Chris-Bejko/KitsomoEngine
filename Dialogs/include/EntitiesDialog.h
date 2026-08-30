#pragma once

#include "Dialog.h"
#include "Logger.h"
#include <deque>
#include <string>
#include <imgui.h>
#include "ConsoleManager.h"

class Entity;
class EntitiesDialog : public Dialog
{
public:
    EntitiesDialog();

    void Draw() override;
    void Open() override;
    void Close() override;
private:
    EntityManager* manager = nullptr;
    void DisplayEntities();
    void DisplayEntityNode(Entity *e);
	Entity *selectedEntity = nullptr;
	Entity *lastClickedEntity = nullptr;
	float lastClickTime = 0.f;
	Entity *dragHoveredEntity = nullptr;

};