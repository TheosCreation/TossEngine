#pragma once
#include "Component.h"
#include "Sound.h"
#include "AudioEngine.h"

class TOSSENGINE_API AudioSource : public Component
{
public:
    void onStart() override;
    void onUpdate() override;
    void onDestroy() override;

    void OnInspectorGUI() override;

    void Play();
    void Pause();
    void Stop();

    bool IsPlaying() const;
    bool IsPaused() const;

    void SetClip(SoundPtr _clip);
private:
    void update3DFromTransform();

    SoundPtr       m_clip = nullptr;
    FMOD::Channel* m_channel = nullptr;
    Vector3 m_lastPos{};
    bool    m_haveLastPos = false;

    bool  m_loop = false;
    bool  m_is3D = true;
    float m_volume = 1.0f;
    bool  m_playOnStart = false;

    SERIALIZABLE_MEMBERS(m_clip, m_loop, m_is3D, m_volume, m_playOnStart)
};

REGISTER_COMPONENT(AudioSource);