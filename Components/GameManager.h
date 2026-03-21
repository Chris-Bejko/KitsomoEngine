#pragma once
#include "../SerializableScript.h"
#include "../Components/UIText.h"
#include "../Components/UIButton.h"

class GameManager : public SerializableScript
{
public:
    bool Init() override
    {
        Field("scoreText", scoreText);   // UIText*
        Field("button",    button);      // UIButton*
        Field("player",    player);      // Entity*
        Field("score",     score);       // int
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
    }


private:
    UIText*   scoreText = nullptr;
    UIButton* button    = nullptr;
    Entity*   player    = nullptr;
    int       score     = 0;
};