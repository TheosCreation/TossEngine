/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : Sound.h
Description : Sound class representing a sound resource (2D or 3D) in TossEngine.
Author : Theo Morris
***/

#pragma once

#include "Resource.h"
#include <string>
#include <map>

/**
 * @class Sound
 * @brief Represents a sound resource, supporting 2D/3D positioning, looping, volume, and reverb control.
 */
class TOSSENGINE_API Sound : public Resource
{
public:
    /**
     * @brief Constructor for creating a Sound from a description.
     * @param desc The descriptor containing sound settings.
     * @param uniqueID Unique identifier for the sound.
     * @param manager Pointer to the ResourceManager managing this sound.
     */
    Sound(const SoundDesc& desc, const std::string& uniqueID, ResourceManager* manager);

    /**
     * @brief Constructor for creating a Sound from deserialization.
     * @param uid Unique identifier.
     * @param mgr Pointer to ResourceManager.
     */
    Sound(const std::string& uid, ResourceManager* mgr);

    void OnInspectorGUI() override;
    void onCreateLate() override;

    // Getters
    bool is3D() const { return m_is3D; }
    bool isLoop() const { return m_isLoop; }
    float getVolume() const { return m_volume; }
    float getReverbAmount() const { return m_desc.reverbAmount; }
    bool isLoaded() const { return m_loaded; }
    float getX() const { return m_position.x; }
    float getY() const { return m_position.y; }
    float getZ() const { return m_position.z; }

    // Setters
    void setVolume(float newVolume) { m_desc.volume = newVolume; }
    void setReverbAmount(float newReverbAmount) { m_desc.reverbAmount = newReverbAmount; }
    void setLoaded(bool isLoaded) { m_loaded = isLoaded; }

private:
    SoundDesc m_desc = {};       //!< Sound descriptor.
    bool m_is3D = false;          //!< Whether the sound is spatialized (3D).
    bool m_isLoop = false;        //!< Whether the sound loops.
    float m_volume = 1.0f;        //!< Volume of the sound [0.0 - 1.0].
    bool m_loaded = false;        //!< Whether the sound is loaded into memory.

    Vector3 m_position = Vector3(); //!< 3D position of the sound.

    SERIALIZABLE_MEMBERS(m_path, m_is3D, m_isLoop, m_volume, m_desc.reverbAmount)
};

REGISTER_RESOURCE(Sound)

// JSON Serialization Helpers
inline void to_json(json& j, SoundPtr const& sound) {
    if (sound) {
        j = json{ { "id", sound->getUniqueID() } };
    }
    else {
        j = nullptr;
    }
}

inline void from_json(json const& j, SoundPtr& sound) {
    if (j.contains("id")) {
        sound = ResourceManager::GetInstance().get<Sound>(j["id"].get<std::string>());
    }
}