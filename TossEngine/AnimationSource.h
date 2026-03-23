#pragma once
#include "Component.h"
#include "Resources/Animation.h"

class TOSSENGINE_API AnimationSource : public Component
{
public:
    void onCreate() override;
    void onStart() override;
    void onUpdate() override;
    void OnInspectorGUI() override;

    void Play(const std::string& clipName, bool loop = true);
    void Stop();

private:
    const AnimationClipData* GetCurrentClip() const;
    void ApplyCurrentPose();

private:
    AnimationPtr m_animation;
    std::string m_clipName;
    float m_currentTime = 0.0f;
    bool m_isPlaying = false;
    bool m_loop = true;
    bool m_playOnStart = false;

    SERIALIZABLE_MEMBERS(m_animation, m_clipName, m_currentTime, m_playOnStart, m_loop)
};

REGISTER_COMPONENT(AnimationSource);
