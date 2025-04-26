#include "UiManager.h"

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
}

void UiManager::UpdateLevelTimer(float secondsLeft)
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
