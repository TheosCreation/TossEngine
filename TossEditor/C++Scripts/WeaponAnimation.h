#pragma once
#include <TossEngine.h>

class PlayerController;

class WeaponAnimation : public Component
{
public:
    void OnInspectorGUI() override;

    void onStart() override;
    void onUpdate() override;
    void UpdateBobbingTargets();

private:
    float m_bobbingHorizontalOffset = 0.08f;
    float m_bobbingverticalOffset = 0.025f;
    float m_bobbingMoveSpeed = 6.0f;
    float m_returnToCenterSpeed = 4.0f;
    float m_velocityOffsetMultiplier = 0.005f;
    float m_maxVelocityOffset = 0.5f;
    Vector3 originalPos;
    Vector3 bobbingPosition;  // The base animated position (bobbing)
    Vector3 rightPos;
    Vector3 leftPos;
    Vector3 target;
    bool backToStart;
    float speed;
    Vector3 currentOffset;
    PlayerController* m_playerController;

    SERIALIZABLE_MEMBERS(m_bobbingHorizontalOffset, m_bobbingverticalOffset, m_velocityOffsetMultiplier, m_maxVelocityOffset, m_bobbingMoveSpeed, m_returnToCenterSpeed)
};

REGISTER_COMPONENT(WeaponAnimation)
