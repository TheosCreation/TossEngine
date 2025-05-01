#include "PauseManager.h"

#include "UiManager.h"

PauseManager* PauseManager::Instance = nullptr;

void PauseManager::onCreate()
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

void PauseManager::onDestroy()
{
    if (Instance == this)
    {
        Instance = nullptr;
    }
}

void PauseManager::SetPaused(bool paused, bool openPauseMenu)
{
    m_isPaused = paused;
    if (openPauseMenu)
    {
        UiManager::Get()->SetPauseMenu(m_isPaused);
    }
    if (m_isPaused)
    {
        Time::TimeScale = 0.0f;
    }
    else
    {
        Time::TimeScale = 1.0f;
    }
    InputManager::GetInstance().enablePlayMode(!m_isPaused, false);

    Debug::Log("Pause status: " + ToString(m_isPaused) + ", Time Scale: " + ToString(Time::TimeScale));
}

void PauseManager::TogglePaused()
{
    SetPaused(!m_isPaused);
}
