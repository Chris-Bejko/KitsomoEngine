#include "GameManager.h"
#include "Entity.h"
#include "ComponentRegistry.h"

DECLARE_COMPONENT_RULES(GameManager, false)
REGISTER_SERIALIZABLE_COMPONENT(GameManager)

void GameManager::SetGameState(int _gameState)
{
    gameState = _gameState;
    if (!(environment && endUI && startUI))
        return;

    if (_gameState == 0)
    {
        score = 0;
        if (scoreText)
            scoreText->SetText("Score: 0");
        environment->SetActive(false);
        endUI->SetActive(false);
        startUI->SetActive(true);
    }
    else if (gameState == 1)
    {
        endUI->SetActive(false);
        startUI->SetActive(false);
        environment->SetActive(true);
    }
    else if (gameState == 2)
    {
        finalScoreText->SetText("Final score: " + std::to_string(this->score));
        if (score > highScore)
        {
            highScore = score;
            PlayerPrefs::get().SetInt("highScore", highScore);
            PlayerPrefs::get().Save();
            if (highScoreText)
                highScoreText->SetText("New High score!: " + std::to_string(highScore));
        }
        else
        {
            if (PlayerPrefs::get().HasKey("highScore"))
                if (highScoreText)
                    highScoreText->SetText("High score: " + std::to_string(highScore));
        }
        startUI->SetActive(false);
        environment->SetActive(false);
        endUI->SetActive(true);
    }
    else
    {
        LOG_WARNING("Invalid game state for GameManager: ", _gameState, ". Valid options 0-2!");
    }
}

void GameManager::AddToScore(int score)
{
    this->score += score;
    if (!scoreText)
        return;

    scoreText->SetText("Score: " + std::to_string(this->score));
}