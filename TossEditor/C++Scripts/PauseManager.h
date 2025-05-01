#pragma once
#include <TossEngine.h>

class PauseManager : public Component 
{
public:
    static PauseManager* Get() { return Instance; }

    void onCreate() override;
    void onDestroy() override;
    void SetPaused(bool paused, bool openPauseMenu = true);
    void TogglePaused();

private:
    bool m_isPaused = false;

    static PauseManager* Instance;
};

REGISTER_COMPONENT(PauseManager);
