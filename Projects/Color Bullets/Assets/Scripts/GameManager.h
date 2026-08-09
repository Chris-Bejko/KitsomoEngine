#pragma once
#include "SerializableScript.h"
#include "Logger.h"
#include "UIText.h"
#include "UIButton.h"
#include "PlayerPrefs.h"

class GameManager : public SerializableScript
{
public:
    bool Init() override
    {
        Field("scoreText", scoreText);
        Field("player", player);
        Field("score", score);
        Field("finalScore", finalScoreText);
        Field("environment", environment);
        Field("GameState", gameState);
        Field("startUI", startUI);
        Field("endUI", endUI);
        Field("playButton", playButton);
        Field("playAgainButton", playAgainButton);
        Field("mainMenuButton", mainMenuButton);
        Field("highScore", highScore);  
        Field("highScoreText", highScoreText);
        return true;
    }

    void Awake() override
    {
        if(PlayerPrefs::get().HasKey("highScore"))
            highScore = PlayerPrefs::get().GetInt("highScore");


        if (scoreText)
            scoreText->SetText("Score: 0");

        if (playButton)
            playButton->AddOnClick([this]()
                                   { SetGameState(1); });

        if (mainMenuButton)
            mainMenuButton->AddOnClick([this]()
                                       { SetGameState(0); });

        if (playAgainButton)
            playAgainButton->AddOnClick([this]()
                                        { 
            score = 0;
            if (scoreText)
                scoreText->SetText("Score: 0");
            SetGameState(1); });

        SetGameState(0);
    }

    void SetGameState(int _gameState);

    void AddToScore(int score);
    int GetGameState() {return gameState; }

private:
    UIText *scoreText = nullptr;
    UIText *finalScoreText = nullptr;
    UIText *highScoreText = nullptr;


    UIButton *playButton = nullptr;
    UIButton *mainMenuButton = nullptr;
    UIButton *playAgainButton = nullptr;

    Entity *player = nullptr;
    Entity *environment = nullptr;
    Entity *startUI = nullptr;
    Entity *endUI = nullptr;

    int gameState;
    std::vector<int> ExampleSerializedVector;
    std::vector<bool> BoolsList;
    int score = 0;
    int highScore = 0;
};