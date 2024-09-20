#include <iostream>
#include <stdio.h>
#include <SFML/Graphics.hpp>
#include "Engine.h"
#include "imgui.h"
#include "imgui-SFML.h"

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
    std::cout << "Loop Broken" << std::endl;
    ImGui::SFML::Shutdown();

    return 0;
}
