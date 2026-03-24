#pragma once
#include "../SerializableScript.h"
#include "../Color.h"
#include "Sprite.h"
#include "Rigidbody.h"
#include "GameManager.h"

class Enemy : public SerializableScript
{
public:
    bool Init() override
    {
        Field("moveSpeed", moveSpeed);
        Field("colorString", colorString);
        Field("rotationSpeed", rotationSpeed);
        return true;
    }

    void Awake() override
    {
        // Assign random color
        std::vector<std::string> colors = {"Green", "Blue", "Magenta", "Cyan"};
        colorString = colors[rand() % colors.size()];

        if (entity->HasComponent<Sprite>())
        {
            SetColor(colorString);
        }

        gameManager = FindObjectOfType<GameManager>();
        if(!gameManager) 
        {
            LOG_WARNING("No game manager found for enemy with guid: ", entity->GetGUID(), ". Destroying");
            entity->Destroy();
        }
    }

    void update(float dt) override
    {
        if (!playerEntity)
            return;

        if(gameManager->GetGameState() != 1)
            return;

        Vector2F myPos = entity->transform->GetWorldPosition();
        Vector2F playerPos = playerEntity->transform->GetWorldPosition();
        Vector2F dir = (playerPos - myPos).normalized();

        // Rotate towards player
        float targetAngle = atan2(dir.y, dir.x) * 180.f / 3.14159f + 90.f;

        // Smooth rotation
        float currentAngle = entity->transform->rotation;
        float angleDiff = targetAngle - currentAngle;

        // Normalize angle difference to [-180, 180]
        while (angleDiff > 180.f)
            angleDiff -= 360.f;
        while (angleDiff < -180.f)
            angleDiff += 360.f;

        entity->transform->rotation += angleDiff * rotationSpeed * dt;

        // Move towards player
        entity->transform->position += dir * moveSpeed * dt;
    }

    void OnTriggerEnter(Collider &other) override
    {
        // Check if hit by bullet
        if (other.entity->HasComponent<Bullet>())
        {
            auto &bullet = other.entity->GetComponent<Bullet>();
            if (bullet.GetColorString() == colorString)
            {
                // Correct color - enemy dies
                LOG_INFO("Enemy killed by correct color!");
                gameManager->AddToScore(1);
                entity->Destroy();
                other.entity->Destroy();
            }
            else
            {
                // Wrong color - player loses
                LOG_INFO("Wrong color! Game over!");
                gameManager->SetGameState(2);
            }
        }
        if (other.entity->HasComponent<FloorSquare>())
        {
            auto &floor = other.entity->GetComponent<FloorSquare>();
            std::string floorColor = floor.GetColorEnum().SerializeColor();
            if (floorColor == colorString)
                return;
            SetColor(floorColor);
        }
    }

    void SetColor(std::string color)
    {
        colorString = color;
        Color c;
        c.SetColor(colorString);
        entity->GetComponent<Sprite>().SetColor(c);
    }

    std::string GetColorString() { return colorString; }
    void SetPlayerEntity(Entity *p) { playerEntity = p; }
private:
    float moveSpeed = 100.f;
    float rotationSpeed = 5.f;
    std::string colorString = "Red";
    Entity *playerEntity = nullptr;
    GameManager *gameManager = nullptr;
};