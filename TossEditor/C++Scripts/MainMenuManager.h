#pragma once
#include <TossEngine.h>

class MainMenuManager : public Component 
{
public:
    static MainMenuManager* Get() { return Instance; }

    void OnInspectorGUI() override;
    void onStart() override;
    void Play();
    void Quit();

private:
    GameObjectPtr playButtonObject = nullptr;
    Button* playButton = nullptr;
    GameObjectPtr quitButtonObject = nullptr;
    Button* quitButton = nullptr;

    static MainMenuManager* Instance;

    SERIALIZABLE_MEMBERS(playButtonObject, quitButtonObject)
};

REGISTER_COMPONENT(MainMenuManager);
