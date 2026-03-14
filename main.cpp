#include <iostream>
#include <stdio.h>
#include <SFML/Graphics.hpp>
#include "Engine.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include "Logger.h"

int main()
{

    auto& engine = Engine::get();
    engine.Init();

    while (engine.IsRunning())
    {
        engine.Events();
        engine.Update();
        engine.Render();
    }
    LOG_INFO("EXITING");
    ImGui::SFML::Shutdown();

    return 0;
}
