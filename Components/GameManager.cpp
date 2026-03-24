#include "GameManager.h"
#include "Entity.h"

void GameManager::SetGameState(int _gameState)
{
    gameState = _gameState;
    if(!(environment && endUI && startUI)) return;

    if (_gameState == 0)
    {
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
    else if(gameState == 2)
    {
        finalScoreText->SetText("Final score: " + std::to_string(this->score));
        startUI->SetActive(false);
        environment->SetActive(false);
        endUI->SetActive(true);
    }
    else
    {
        LOG_WARNING("Invalid game state for GameManager: ", _gameState , ". Valid options 0-2!");
    }
}

void GameManager::AddToScore(int score)
{
    this->score += score;
    if (!scoreText) 
        return;
    
    scoreText->SetText("Score: " + std::to_string(this->score));
}