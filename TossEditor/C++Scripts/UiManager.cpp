#include "UiManager.h"

#include "PauseManager.h"

UiManager* UiManager::Instance = nullptr;

void UiManager::onCreate()
{
    if (Instance == nullptr)
    {
        Instance = this;
    }
    else if (Instance != this)
    {
        Destroy(m_owner);
    }
}

void UiManager::onStart()
{
    m_levelTimeText = m_levelTimeTextObject->getComponent<Text>();
    m_playButton = m_playButtonObject->getComponent<Button>();
    m_playButton->onClick = [this]() {
        PauseManager::Get()->SetPaused(false);
        };

    m_quitButton = m_quitButtonObject->getComponent<Button>();
    m_quitButton->onClick = [this]() {
        PauseManager::Get()->SetPaused(false);
        InputManager::GetInstance().enablePlayMode(false, false);
        TossEngine::GetInstance().OpenScene("MainMenu");
        };

    m_gameOverplayButton = m_gameOverplayButtonObject->getComponent<Button>();
    m_gameOverplayButton->onClick = [this]() {
        PauseManager::Get()->SetPaused(false);
        TossEngine::GetInstance().OpenScene("Scene1");
        };

    m_gameOverquitButton = m_gameOverquitButtonObject->getComponent<Button>();
    m_gameOverquitButton->onClick = [this]() {
        PauseManager::Get()->SetPaused(false);
        InputManager::GetInstance().enablePlayMode(false, false);
        TossEngine::GetInstance().OpenScene("MainMenu");
        };


    m_gameLossPlayButton = m_gameLossPlayButtonObject->getComponent<Button>();
    m_gameLossPlayButton->onClick = [this]() {
        PauseManager::Get()->SetPaused(false);
        TossEngine::GetInstance().OpenScene("Scene1");
        };

    m_gameLossQuitButton = m_gameLossQuitButtonObject->getComponent<Button>();
    m_gameLossQuitButton->onClick = [this]() {
        PauseManager::Get()->SetPaused(false);
        InputManager::GetInstance().enablePlayMode(false, false);
        TossEngine::GetInstance().OpenScene("MainMenu");
        };
}

void UiManager::onDestroy()
{
    if (Instance == this)
    {
        Instance = nullptr;
    }
}

void UiManager::OnInspectorGUI()
{
    Component::OnInspectorGUI();

    GameObjectAssignableField(m_levelTimeTextObject, "Level Time Text");
    GameObjectAssignableField(m_crosshairImageObject, "Crosshair Image Object");
    //pause menu
    GameObjectAssignableField(m_pauseMenuObject, "Pause Menu Object");
    GameObjectAssignableField(m_playButtonObject, "Play Button Object");
    GameObjectAssignableField(m_quitButtonObject, "Quit Button Object");
    //game over screen
    GameObjectAssignableField(m_gameWinMenuObject, "GameOverScreen Object");
    GameObjectAssignableField(m_gameOverplayButtonObject, "Play Again GameOverScreen Button Object");
    GameObjectAssignableField(m_gameOverquitButtonObject, "Quit Button GameOverScreen Object");
    //game loss screen
    GameObjectAssignableField(m_gameLossMenuObject, "Game Loss Screen Object");
    GameObjectAssignableField(m_gameLossPlayButtonObject, "Play Again Game Loss Button Object");
    GameObjectAssignableField(m_gameLossQuitButtonObject, "Quit Button Game Loss Object");
}

void UiManager::UpdateLevelTimer(float secondsLeft) const
{
    if (m_levelTimeText)
    {
        m_levelTimeText->SetText(ToString(secondsLeft));
    }
    else
    {
        Debug::Log("Level Text is null");
    }
}

void UiManager::SetCrosshair(bool active) const
{
    m_crosshairImageObject->isActive = active;
}

void UiManager::SetPauseMenu(bool active) const
{
    m_pauseMenuObject->isActive = active;
}

void UiManager::SetGameWin(bool active) const
{
    m_gameWinMenuObject->isActive = active;
}

void UiManager::SetGameLoss(bool active) const
{
    m_gameLossMenuObject->isActive = active;
}