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
    m_ammoReserveText = m_ammoReserveTextObject->getComponent<Text>();
    m_ammoLeftText = m_ammoLeftTextObject->getComponent<Text>();
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
    GameObjectAssignableField(m_ammoReserveTextObject, "Ammo Reserve Text");
    GameObjectAssignableField(m_ammoLeftTextObject, "Ammo Left Text");
}

void UiManager::UpdateAmmoText(int ammoLeft, int ammoReserve)
{
    m_ammoLeftText->SetText(ToString(ammoLeft));
    m_ammoReserveText->SetText(ToString(ammoReserve));
}

void UiManager::UpdateLevelTimer(float secondsLeft)
{
    m_levelTimeText->SetText(ToString(secondsLeft));
}
