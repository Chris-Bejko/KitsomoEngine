#pragma once
#include "../SerializableScript.h"
#include "../Components/UIText.h"
#include "../Components/UIButton.h"
#include "../Logger.h"
#include "FloorSquare.h"
class GameManager : public SerializableScript
{
public:
    bool Init() override
    {
        Field("scoreText", scoreText);   // UIText*
        Field("button",    button);      // UIButton*
        Field("player",    player);      // Entity*
        Field("score",     score);       // int
        Field("FloorSquare", floorSquare);
        return true;
    }

    void Awake() override
    {
        // Pointers already resolved by ResolvePointers()!
        if (button)
            button->AddOnClick([this]() { 
                score++; 
                if (scoreText)
                    scoreText->SetText("Score: " + std::to_string(score));
            });

        if (player){
            LOG_INFO("Player retrieved with guid: ", player->GetGUID());
        }

        if(floorSquare){
           floorSquare->Config(Vector2F(0,0), Color(ColorEnum::Magenta));
        }
    }


private:
    UIText*   scoreText = nullptr;
    UIButton* button    = nullptr;
    Entity*   player    = nullptr;
    FloorSquare* floorSquare = nullptr;
    int       score     = 0;
};