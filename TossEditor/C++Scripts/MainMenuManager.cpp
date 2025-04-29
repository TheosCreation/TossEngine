#include "MainMenuManager.h"

MainMenuManager* MainMenuManager::Instance = nullptr;

void MainMenuManager::OnInspectorGUI()
{
    Component::OnInspectorGUI();
    GameObjectAssignableField(playButtonObject, "Play Button");
    GameObjectAssignableField(quitButtonObject, "Quit Button");
}

void MainMenuManager::onStart()
{
    playButton = playButtonObject->getComponent<Button>();
    if (playButton)
    {
        playButton->onClick = [this]() { this->Play(); };
    }
    quitButton = quitButtonObject->getComponent<Button>();
    if (quitButton)
    {
        quitButton->onClick = [this]() { this->Quit(); };
    }
}

void MainMenuManager::Play()
{
    Debug::Log("Play Function Called");
    TossEngine::GetInstance().OpenScene("Scene1");
}

void MainMenuManager::Quit()
{
    TossEngine::GetInstance().Quit();
}
