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
        TossEngine::GetInstance().OpenScene("MainMenu");
        };

    m_gameOverplayButton = m_gameOverplayButtonObject->getComponent<Button>();
    m_gameOverplayButton->onClick = [this]() {
        TossEngine::GetInstance().OpenScene("Scene1");
        };

    m_gameOverquitButton = m_gameOverplayButtonObject->getComponent<Button>();
    m_gameOverquitButton->onClick = [this]() {
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
    GameObjectAssignableField(m_pauseMenuObject, "Pause Menu Object");
    GameObjectAssignableField(m_playButtonObject, "Play Button Object");
    GameObjectAssignableField(m_quitButtonObject, "Quit Button Object");
    GameObjectAssignableField(m_gameOverMenuObject, "GameOverScreen Object");
    GameObjectAssignableField(m_gameOverquitButtonObject, "Play Again GameOverScreen Button Object");
    GameObjectAssignableField(m_gameOverplayButtonObject, "Quit Button GameOverScreen Object");
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
    m_gameOverMenuObject->isActive = active;
}
