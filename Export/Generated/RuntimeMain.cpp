#include "Engine.h"
#include "Logger.h"

int main()
{
    Engine& engine = Engine::get();

    engine.InitRuntime(
        ".",
        "MainNew"
    );

    while (engine.IsRunning())
    {
        engine.Events();
        engine.UpdateRuntime();
        engine.RenderRuntime();
    }

    return 0;
}
