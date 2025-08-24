#pragma once
///
/// @file AudioEngine.h
/// 
/// FMOD wrapper which loads sound files (.wav, .mp3, .ogg etc) and FMOD soundbanks (.bank files)
/// and supports looping or one-shot playback in stereo, as well as customizable 3D positional audio
/// Implements the FMOD Studio and FMOD Core API's to allow audio file-based implementations,
/// alongside/in addition to use of FMOD Studio Sound Banks.
///
/// @author Theo Morris
/// @dependencies FMOD Studio & Core
/// 
#include <fmod_studio.hpp>
#include <fmod.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include "Sound.h"

/**
 * Class that handles the process of loading and playing sounds by wrapping FMOD's functionality.
 * Deals with all FMOD calls so that FMOD-specific code does not need to be used outside this class.
 * Only one AudioEngine should be constructed for an application.
 */
class TOSSENGINE_API AudioEngine {
public:

    /**
     * @brief Provides access to the singleton instance of GraphicsEngine.
     * @return A reference to the GraphicsEngine instance.
     */
    static AudioEngine& GetInstance()
    {
        static AudioEngine instance;
        return instance;
    }

    // Delete the copy constructor and assignment operator to prevent copying
    AudioEngine(const AudioEngine& other) = delete;
    AudioEngine& operator=(const AudioEngine& other) = delete;

    void Init();

    /**
     * Method that is called to CleanUp the audio engine after use.
     */
    void CleanUp();

    /**
    * Method which should be called every frame of the game loop
    */
    void Update();
    
    /**
     * Loads a sound from disk using provided settings
     * Prepares for later playback with playSound()
     * Only reads the audio file and loads into the audio engine
     * if the sound file has already been added to the cache
     */
    void loadSound(Sound& sound);
    void loadSound(SoundPtr& soundp);
    FMOD::Sound* getFmodSound(const std::string& uid) const;

    void set3DListenerPosition(float px, float py, float pz,
                               float fx, float fy, float fz,
                               float ux, float uy, float uz);

    void set3DListener(Transform& transform);

    // Mute
    void muteAllSounds();
    void unmuteAllSound();
    bool isMuted();

    // Accessor for low level system if needed by components
    FMOD::System* lowLevel() const { return lowLevelSystem; }

private:  

    /**
     * @brief Private constructor to prevent external instantiation.
     */
    AudioEngine() = default;

    /**
     * @brief Private destructor to prevent external deletion.
     */
    ~AudioEngine() = default;

    //void initReverb();

    FMOD::Studio::System* studioSystem = nullptr;
    FMOD::System* lowLevelSystem = nullptr;
    FMOD::ChannelGroup* mastergroup = nullptr;

    std::map<std::string, FMOD::Sound*> sounds;
    bool muted = false;
    bool m_initialized = false;

    // listener cached
    FMOD_VECTOR listenerpos{ 0,0,-1 };
    FMOD_VECTOR forward{ 0,0,1 };
    FMOD_VECTOR up{ 0,1,0 };

    // Low-level system reverb TODO add multi-reverb support
    //FMOD::Reverb3D* reverb;

    // Reverb origin position
    //FMOD_VECTOR revPos = { 0.0f, 0.0f, 0.0f };

    // reverb min, max distances
    //float revMinDist = 10.0f, revMaxDist = 50.0f;

};
