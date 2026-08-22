// AUTO-GENERATED - DO NOT EDIT
#include "HotReloading/include/ProjectComponentRegistration.h"
#include "HotReloading/include/ProjectModuleAPI.h"

#include "Bullet.h"
#include "Enemy.h"
#include "EnemySpawner.h"
#include "FloorSquare.h"
#include "GameManager.h"
#include "Player.h"

extern "C" __declspec(dllexport) void RegisterProjectComponents(RegisterProjectComponentFn registerFn)
{
    RegisterProjectComponent<Bullet>(registerFn, "Bullet");
    RegisterProjectComponent<Enemy>(registerFn, "Enemy");
    RegisterProjectComponent<EnemySpawner>(registerFn, "EnemySpawner");
    RegisterProjectComponent<FloorSquare>(registerFn, "FloorSquare");
    RegisterProjectComponent<GameManager>(registerFn, "GameManager");
    RegisterProjectComponent<Player>(registerFn, "Player");
}
