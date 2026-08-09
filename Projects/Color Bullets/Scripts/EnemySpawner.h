#pragma once
#include "SerializableScript.h"
#include "Entity.h"
#include "Engine.h"
#include "Enemy.h"

class EnemySpawner : public SerializableScript
{
public:
    bool Init() override
    {
        Field("spawnInterval", spawnInterval);
        Field("enemyPrefab", enemyPrefab);
        Field("player", player);
        Field("GameManager", gameManager);
        Field("Environment", environment);
        return true;
    }

    void Awake() override
    {
        gameManager = FindObjectOfType<GameManager>();
    }
    void update(float dt) override
    {
        if (!player || !gameManager || gameManager->GetGameState() != 1)
            return;

        timer += dt;
        if (timer >= spawnInterval)
        {
            timer = 0.f;
            SpawnEnemy();
        }
    }

private:
    float spawnInterval = 3.f;
    std::string enemyPrefab = "Enemy";
    Entity *player = nullptr;
    Entity* environment = nullptr;
    float timer = 0.f;
    GameManager *gameManager = nullptr;
    void SpawnEnemy()
    {
        if (!gameManager)
            return;
        if (gameManager->GetGameState() != 1)
            return;

        if (! environment)
            return;
        // Spawn at random edge of screen
        auto windowSize = Engine::get().GetWindow().getSize();
        sf::View view = Engine::get().GetWindow().getView();

        float edge = (float)(rand() % 4);
        Vector2F spawnPos;
        float margin = 100.f;

        sf::Vector2f center = view.getCenter();
        sf::Vector2f size = view.getSize();

        switch ((int)edge)
        {
        case 0: // top
            spawnPos = Vector2F(
                center.x + (rand() % (int)size.x) - size.x * 0.5f,
                center.y - size.y * 0.5f - margin);
            break;
        case 1: // bottom
            spawnPos = Vector2F(
                center.x + (rand() % (int)size.x) - size.x * 0.5f,
                center.y + size.y * 0.5f + margin);
            break;
        case 2: // left
            spawnPos = Vector2F(
                center.x - size.x * 0.5f - margin,
                center.y + (rand() % (int)size.y) - size.y * 0.5f);
            break;
        case 3: // right
            spawnPos = Vector2F(
                center.x + size.x * 0.5f + margin,
                center.y + (rand() % (int)size.y) - size.y * 0.5f);
            break;
        }

        Entity *enemy = Engine::get().SpawnPrefab(enemyPrefab, spawnPos);
        if (enemy && enemy->HasComponent<Enemy>())
        {
            enemy->SetParent(environment);
            auto &spawned = enemy->GetComponent<Enemy>();
            spawned.SetPlayerEntity(player);
        }
    }
};