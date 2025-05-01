#pragma once
#include <TossEngine.h>

class UiManager : public Component 
{
public:
    static UiManager* Get() { return Instance; }

    void onCreate() override;
    void onStart() override;
    void onDestroy() override;
    void OnInspectorGUI() override;

    void UpdateLevelTimer(float secondsLeft) const;

    void SetCrosshair(bool active) const;
    void SetPauseMenu(bool active) const;
    void SetGameWin(bool active) const;


private:
    GameObjectPtr m_levelTimeTextObject = nullptr;
    GameObjectPtr m_crosshairImageObject = nullptr;
    GameObjectPtr m_pauseMenuObject = nullptr;
    GameObjectPtr m_quitButtonObject = nullptr;
    GameObjectPtr m_playButtonObject = nullptr;
    GameObjectPtr m_gameOverMenuObject = nullptr;
    GameObjectPtr m_gameOverquitButtonObject = nullptr;
    GameObjectPtr m_gameOverplayButtonObject = nullptr;

    Text* m_levelTimeText = nullptr;
    Button* m_playButton = nullptr;
    Button* m_quitButton = nullptr;
    Button* m_gameOverplayButton = nullptr;
    Button* m_gameOverquitButton = nullptr;

    static UiManager* Instance;

    SERIALIZABLE_MEMBERS(m_levelTimeTextObject, m_crosshairImageObject, m_pauseMenuObject, m_playButtonObject, m_quitButtonObject, m_gameOverMenuObject, m_gameOverquitButtonObject, m_gameOverplayButtonObject)
};

REGISTER_COMPONENT(UiManager);
