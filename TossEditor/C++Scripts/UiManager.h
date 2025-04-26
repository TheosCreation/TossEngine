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

    void UpdateAmmoText(int ammoLeft, int ammoReserve);
    void UpdateLevelTimer(float secondsLeft);


private:
    GameObjectPtr m_levelTimeTextObject = nullptr;
    GameObjectPtr m_ammoLeftTextObject = nullptr;
    GameObjectPtr m_ammoReserveTextObject = nullptr;

    Text* m_levelTimeText = nullptr;
    Text* m_ammoReserveText = nullptr;
    Text* m_ammoLeftText = nullptr;

    static UiManager* Instance;

    SERIALIZABLE_MEMBERS(m_levelTimeTextObject, m_ammoLeftTextObject, m_ammoReserveTextObject)
};

REGISTER_COMPONENT(UiManager);
