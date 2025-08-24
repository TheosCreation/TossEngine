#include "AudioSource.h"

void AudioSource::onStart()
{
    if (m_playOnStart && m_clip) Play();
}

void AudioSource::onUpdate()
{
    if (m_channel && m_is3D) update3DFromTransform();
}

void AudioSource::onDestroy()
{
    Stop();
}

void AudioSource::OnInspectorGUI()
{
    Component::OnInspectorGUI();

    ImGui::Checkbox("Play On Start", &m_playOnStart);
    ImGui::Checkbox("Is Looping", &m_loop);
    ImGui::Checkbox("Is 3D", &m_is3D);
    ImGui::DragFloat("Volume", &m_volume, 0.01f, 0, 1);
    ResourceAssignableField(m_clip, "Audio Clip");
}

void AudioSource::Play()
{
    if (!m_clip) return;

    AudioEngine& engine = AudioEngine::GetInstance();
    engine.loadSound(m_clip);
    FMOD::Sound* fmodSound = engine.getFmodSound(m_clip->getUniqueID());
    if (!fmodSound) return;

    // Ensure loop flag
    fmodSound->setMode(m_loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);

    // Start paused to configure channel
    FMOD_RESULT r = engine.lowLevel()->playSound(fmodSound, nullptr, true, &m_channel);
    if (r != FMOD_OK || !m_channel) return;

    m_channel->setVolume(m_volume);
    if (m_is3D) {
        // 3D mode for channel
        m_channel->setMode((m_loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF) | FMOD_3D);
        update3DFromTransform();
    }
    else {
        m_channel->setMode((m_loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF) | FMOD_2D);
    }

    m_channel->setPaused(false);
}

void AudioSource::Pause()
{
    if (!m_channel) return;
    bool paused = false;
    m_channel->getPaused(&paused);
    m_channel->setPaused(!paused);
}

void AudioSource::Stop()
{
    if (m_channel) {
        m_channel->stop();
        m_channel = nullptr;
    }
}

bool AudioSource::IsPlaying() const
{
    if (!m_channel) return false;
    bool playing = false;
    m_channel->isPlaying(&playing);
    return playing;
}

bool AudioSource::IsPaused() const
{
    if (!m_channel) return false;
    bool paused = false;
    m_channel->getPaused(&paused);
    return paused;
}

void AudioSource::SetClip(SoundPtr _clip)
{
    m_clip = _clip;
}

// AudioSource.cpp
void AudioSource::update3DFromTransform()
{
    if (!m_channel) return;

    Transform& t = getTransform();
    const Vector3 pos = t.position;
    const Vector3 fwd = t.GetForward();
    const float dt = Time::DeltaTime;

    // Compute velocity for doppler
    Vector3 vel{ 0,0,0 };
    if (m_haveLastPos && dt > 0.0f) {
        vel = (pos - m_lastPos) * (1.0f / dt);
    }
    m_lastPos = pos;
    m_haveLastPos = true;

    // FMOD expects world units to match the system distance factor you set
    FMOD_VECTOR fpos{ pos.x, pos.y, pos.z };
    FMOD_VECTOR fvel{ vel.x, vel.y, vel.z };
    FMOD_RESULT r = m_channel->set3DAttributes(&fpos, &fvel);
    if (r != FMOD_OK) return;

    // Optional: give the source directionality
    //FMOD_VECTOR ffwd{ fwd.x, fwd.y, fwd.z };
    //r = m_channel->set3DConeOrientation(&ffwd);
    //if (r != FMOD_OK) return;
    // Tune as needed (inner angle, outer angle, outer volume)
    // ERRCHECK(m_channel->set3DConeSettings(60.0f, 180.0f, 0.2f));
}