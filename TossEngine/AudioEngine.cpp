/// 
/// @file AudioEngine.cpp
/// @author Theo Morris
///
#include "AudioEngine.h"
#include <fmod_errors.h>
#include <iostream>

// Error checking/debugging function definitions

static void ERRCHECK_fn(FMOD_RESULT r, const char* file, int line) {
    if (r != FMOD_OK) std::cout << "FMOD ERROR [" << line << "] " << r << " - " << FMOD_ErrorString(r) << '\n';
}
#define ERRCHECK(r) ERRCHECK_fn(r,__FILE__,__LINE__)

void AudioEngine::Init()
{
    if (m_initialized) return;

    ERRCHECK(FMOD::Studio::System::create(&studioSystem));
    ERRCHECK(studioSystem->getCoreSystem(&lowLevelSystem));
    ERRCHECK(lowLevelSystem->setSoftwareFormat(44100, FMOD_SPEAKERMODE_STEREO, 0));
    ERRCHECK(studioSystem->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0));
    ERRCHECK(lowLevelSystem->getMasterChannelGroup(&mastergroup));
    //initReverb();

    m_initialized = true;
}

void AudioEngine::CleanUp() {
    if (!m_initialized) return;
    for (auto& kv : sounds) { if (kv.second) kv.second->release(); }
    sounds.clear();
    lowLevelSystem->close();
    studioSystem->release();
    m_initialized = false;
}

void AudioEngine::Update() {
    ERRCHECK(studioSystem->update()); // also updates the low level system
}

void AudioEngine::loadSound(SoundPtr& soundp) {
    if (!soundp) return;
    loadSound(*soundp);
}

FMOD::Sound* AudioEngine::getFmodSound(const std::string& uid) const
{
    auto it = sounds.find(uid);
    return it == sounds.end() ? nullptr : it->second;
}

void AudioEngine::loadSound(Sound& sound)
{
    if (sound.isLoaded()) return;
    FMOD::Sound* fmodSound = nullptr;
    ERRCHECK(lowLevelSystem->createSound(sound.getPath().c_str(), FMOD_DEFAULT, nullptr, &fmodSound));
    sounds.insert({ sound.getUniqueID(), fmodSound });
    sound.setLoaded(true);
}


void AudioEngine::set3DListenerPosition(float posX, float posY, float posZ, float forwardX, float forwardY, float forwardZ, float upX, float upY, float upZ) {
    listenerpos = { posX,     posY,     posZ };
    forward = { forwardX, forwardY, forwardZ };
    up = { upX,      upY,      upZ };
    ERRCHECK(lowLevelSystem->set3DListenerAttributes(0, &listenerpos, nullptr, &forward, &up));
}

void AudioEngine::set3DListener(Transform& transform)
{
    Vector3 pos = transform.position;
    Vector3 fwd = transform.GetForward();
    Vector3 up = transform.GetUp();
    set3DListenerPosition(pos.x, pos.y, pos.z, fwd.x, fwd.y, fwd.z, up.x, up.y, up.z);
}

void AudioEngine::muteAllSounds() {
    ERRCHECK(mastergroup->setMute(true));
    muted = true;
}

void AudioEngine::unmuteAllSound() {
    ERRCHECK(mastergroup->setMute(false));
    muted = false;
}

bool AudioEngine::isMuted() {
    return muted;
}

//void AudioEngine::initReverb() {
//    ERRCHECK(lowLevelSystem->createReverb3D(&reverb));
//    FMOD_REVERB_PROPERTIES prop2 = FMOD_PRESET_CONCERTHALL;
//    ERRCHECK(reverb->setProperties(&prop2));
//    ERRCHECK(reverb->set3DAttributes(&revPos, revMinDist, revMaxDist));
//}