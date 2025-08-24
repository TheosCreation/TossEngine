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

    const std::string& getPath() const { return m_path; }
    bool isLoaded() const { return m_loaded; }
    void setLoaded(bool v) { m_loaded = v; }

private:
    SoundDesc m_desc = {};       //!< Sound descriptor.
    bool m_loaded = false;

    SERIALIZABLE_MEMBERS(m_path)
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