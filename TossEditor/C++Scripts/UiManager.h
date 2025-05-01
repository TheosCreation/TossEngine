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
    void SetGameLoss(bool active) const;


private:
    GameObjectPtr m_levelTimeTextObject = nullptr;
    GameObjectPtr m_crosshairImageObject = nullptr;
    GameObjectPtr m_pauseMenuObject = nullptr;
    GameObjectPtr m_quitButtonObject = nullptr;
    GameObjectPtr m_playButtonObject = nullptr;
    GameObjectPtr m_gameWinMenuObject = nullptr;
    GameObjectPtr m_gameOverquitButtonObject = nullptr;
    GameObjectPtr m_gameOverplayButtonObject = nullptr;

    GameObjectPtr m_gameLossMenuObject = nullptr;
    GameObjectPtr m_gameLossQuitButtonObject = nullptr;
    GameObjectPtr m_gameLossPlayButtonObject = nullptr;

    Text* m_levelTimeText = nullptr;
    Button* m_playButton = nullptr;
    Button* m_quitButton = nullptr;
    Button* m_gameOverplayButton = nullptr;
    Button* m_gameOverquitButton = nullptr;

    Button* m_gameLossPlayButton = nullptr;
    Button* m_gameLossQuitButton = nullptr;

    static UiManager* Instance;

    SERIALIZABLE_MEMBERS(m_levelTimeTextObject, m_crosshairImageObject, m_pauseMenuObject, m_playButtonObject, m_quitButtonObject, 
        m_gameWinMenuObject, m_gameOverquitButtonObject, m_gameOverplayButtonObject,
        m_gameLossMenuObject, m_gameLossQuitButtonObject, m_gameLossPlayButtonObject)
};

REGISTER_COMPONENT(UiManager);
