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
    initReverb();

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
//
//void AudioEngine::playSound(SoundPtr soundInfo) {
//    if (soundInfo->isLoaded()) {
//        //std::cout << "Playing Sound\n";
//        FMOD::Channel* channel;
//        // start play in 'paused' state
//        ERRCHECK(lowLevelSystem->playSound(sounds[soundInfo->getUniqueID()], 0, true /* start paused */, &channel));
//
//
//        //std::cout << "Playing sound at volume " << soundInfo.getVolume() << '\n';
//        channel->setVolume(soundInfo->getVolume());
//
//        if (soundInfo->isLoop()) // add to channel map of sounds currently playing, to stop later
//            loopsPlaying.insert({ soundInfo->getUniqueID(), channel });
//
//        ERRCHECK(channel->setReverbProperties(0, soundInfo->getReverbAmount()));
//
//        // start audio playback
//        ERRCHECK(channel->setPaused(false));
//
//    }
//    else
//        std::cout << "Audio Engine: Can't play, sound was not loaded yet from " << soundInfo->getPath() << '\n';
//
//}
//
//void AudioEngine::stopSound(SoundPtr soundInfo) {
//    if (soundIsPlaying(soundInfo)) {
//        ERRCHECK(loopsPlaying[soundInfo->getUniqueID()]->stop());
//        loopsPlaying.erase(soundInfo->getUniqueID());
//    }
//    else
//        std::cout << "Audio Engine: Can't stop a looping sound that's not playing!\n";
//}
//
//void AudioEngine::updateSoundLoopVolume(SoundPtr& soundInfo, float newVolume, unsigned int fadeSampleLength) {
//    if (soundIsPlaying(soundInfo)) {
//        FMOD::Channel* channel = loopsPlaying[soundInfo->getUniqueID()];
//        if (fadeSampleLength <= 64) // 64 samples is default volume fade out
//            ERRCHECK(channel->setVolume(newVolume));
//        else {
//            bool fadeUp = newVolume > soundInfo->getVolume();
//            // get current audio clock time
//            unsigned long long parentclock = 0;
//            ERRCHECK(channel->getDSPClock(NULL, &parentclock));
//
//            float targetFadeVol = fadeUp ? 1.0f : newVolume;
//
//            if (fadeUp) ERRCHECK(channel->setVolume(newVolume));
//
//            ERRCHECK(channel->addFadePoint(parentclock, soundInfo->getVolume()));
//            ERRCHECK(channel->addFadePoint(parentclock + fadeSampleLength, targetFadeVol));
//            //std::cout << "Current DSP Clock: " << parentclock << ", fade length in samples  = " << fadeSampleLength << "\n";
//        }
//        //std::cout << "Updating with new soundinfo vol \n";
//        soundInfo->setVolume(newVolume); // update the SoundInfo's volume
//    }
//    else
//        std::cout << "AudioEngine: Can't update sound loop volume! (It isn't playing or might not be loaded)\n";
//}
//
//
//
//void AudioEngine::update3DSoundPosition(SoundPtr soundInfo) {
//    if (soundIsPlaying(soundInfo))
//        set3dChannelPosition(soundInfo, loopsPlaying[soundInfo->getUniqueID()]);
//    else
//        std::cout << "Audio Engine: Can't update sound position!\n";
//    // TODO: fix this function and the set 3d position function
//}
//
//bool AudioEngine::soundIsPlaying(SoundPtr soundInfo) {
//    return soundInfo->isLoop() && loopsPlaying.count(soundInfo->getUniqueID());
//}
//
//unsigned int AudioEngine::getSoundLengthInMS(SoundPtr soundInfo) {
//    unsigned int length = 0;
//    if (sounds.count(soundInfo->getUniqueID()))
//        ERRCHECK(sounds[soundInfo->getUniqueID()]->getLength(&length, FMOD_TIMEUNIT_MS));
//    return length;
//}
//
//void AudioEngine::loadFMODStudioBank(const char* filepath) {
//    std::cout << "Audio Engine: Loading FMOD Studio Sound Bank " << filepath << '\n';
//    FMOD::Studio::Bank* bank = NULL;
//    ERRCHECK(studioSystem->loadBankFile(filepath, FMOD_STUDIO_LOAD_BANK_NORMAL, &bank));
//    soundBanks.insert({ filepath, bank });
//}
//
//void AudioEngine::loadFMODStudioEvent(const char* eventName, std::vector<std::pair<const char*, float>> paramsValues) { // std::vector<std::map<const char*, float>> perInstanceParameterValues) {
//    std::cout << "AudioEngine: Loading FMOD Studio Event " << eventName << '\n';
//    FMOD::Studio::EventDescription* eventDescription = NULL;
//    ERRCHECK(studioSystem->getEvent(eventName, &eventDescription));
//    // Create an instance of the event
//    FMOD::Studio::EventInstance* eventInstance = NULL;
//    ERRCHECK(eventDescription->createInstance(&eventInstance));
//    for (const auto& parVal : paramsValues) {
//        std::cout << "AudioEngine: Setting Event Instance Parameter " << parVal.first << "to value: " << parVal.second << '\n';
//        // Set the parameter values of the event instance
//        ERRCHECK(eventInstance->setParameterByName(parVal.first, parVal.second));
//    }
//    eventInstances.insert({ eventName, eventInstance });
//    eventDescriptions.insert({ eventName, eventDescription });
//}
//
//void AudioEngine::setFMODEventParamValue(const char* eventName, const char* parameterName, float value) {
//    if (eventInstances.count(eventName) > 0)
//        ERRCHECK(eventInstances[eventName]->setParameterByName(parameterName, value));
//    else
//        std::cout << "AudioEngine: Event " << eventName << " was not in event instance cache, can't set param \n";
//
//}
//
//void AudioEngine::playEvent(const char* eventName, int instanceIndex) {
//    // printEventInfo(eventDescriptions[eventName]);
//    auto eventInstance = eventInstances[eventName];
//    if (eventInstances.count(eventName) > 0)
//        ERRCHECK(eventInstances[eventName]->start());
//    else
//        std::cout << "AudioEngine: Event " << eventName << " was not in event instance cache, cannot play \n";
//}
//
//void AudioEngine::stopEvent(const char* eventName, int instanceIndex) {
//    if (eventInstances.count(eventName) > 0)
//        ERRCHECK(eventInstances[eventName]->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT));
//    else
//        std::cout << "AudioEngine: Event " << eventName << " was not in event instance cache, cannot stop \n";
//}
//
//void AudioEngine::setEventVolume(const char* eventName, float volume0to1) {
//    std::cout << "AudioEngine: Setting Event Volume\n";
//    ERRCHECK(eventInstances[eventName]->setVolume(volume0to1));
//}
//
//bool AudioEngine::eventIsPlaying(const char* eventName, int instance /*= 0*/) {
//    FMOD_STUDIO_PLAYBACK_STATE playbackState;
//    ERRCHECK(eventInstances[eventName]->getPlaybackState(&playbackState));
//    return playbackState == FMOD_STUDIO_PLAYBACK_PLAYING;
//}


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
//
//// Private definitions 
//bool AudioEngine::soundLoaded(SoundPtr soundInfo) {
//    //std::cout << "Checking sound " << soundInfo.getUniqueID() << " exists\n";
//    return sounds.count(soundInfo->getUniqueID()) > 0;
//}
//
//void AudioEngine::set3dChannelPosition(SoundPtr sound, FMOD::Channel* channel) {
//    FMOD_VECTOR position = { sound->getX() * DISTANCEFACTOR, sound->getY() * DISTANCEFACTOR, sound->getZ() * DISTANCEFACTOR };
//    FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f }; // TODO Add dopplar (velocity) support
//    ERRCHECK(channel->set3DAttributes(&position, &velocity));
//}

void AudioEngine::initReverb() {
    ERRCHECK(lowLevelSystem->createReverb3D(&reverb));
    FMOD_REVERB_PROPERTIES prop2 = FMOD_PRESET_CONCERTHALL;
    ERRCHECK(reverb->setProperties(&prop2));
    ERRCHECK(reverb->set3DAttributes(&revPos, revMinDist, revMaxDist));
}

//
//void AudioEngine::printEventInfo(FMOD::Studio::EventDescription* eventDescription) {
//
//    int params;
//    bool is3D, isOneshot;
//    ERRCHECK(eventDescription->getParameterDescriptionCount(&params));
//    ERRCHECK(eventDescription->is3D(&is3D));
//    ERRCHECK(eventDescription->isOneshot(&isOneshot));
//
//    std::cout << "FMOD EventDescription has " << params << " parameter descriptions, "
//        << (is3D ? " is " : " isn't ") << " 3D, "
//        << (isOneshot ? " is " : " isn't ") << " oneshot, "
//        << (eventDescription->isValid() ? " is " : " isn't ") << " valid."
//        << '\n';
//}