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

    void UpdateLevelTimer(float secondsLeft);


private:
    GameObjectPtr m_levelTimeTextObject = nullptr;

    Text* m_levelTimeText = nullptr;

    static UiManager* Instance;

    SERIALIZABLE_MEMBERS(m_levelTimeTextObject)
};

REGISTER_COMPONENT(UiManager);
