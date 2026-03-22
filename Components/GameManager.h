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
        Field("All Floor Squares", floorSquares);
        Field("ExampleSerializedVector", ExampleSerializedVector);
        Field("Bools", BoolsList);
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

    }


private:
    UIText*   scoreText = nullptr;
    UIButton* button    = nullptr;
    Entity*   player    = nullptr;
    std::vector<FloorSquare*> floorSquares;
    std::vector<int> ExampleSerializedVector;
    std::vector<bool> BoolsList;
    int       score     = 0;
};