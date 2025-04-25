/// 
/// @file SoundInfo.h
/// @author Ross Hoyt
///
#ifndef SOUNDINFO_H
#define SOUNDINFO_H

#include "Resource.h"
#include <string>
#include <map>

class TOSSENGINE_API Sound : public Resource
{
public:
    /**
     * Constructor for SoundInfo.
     * @param uniqueID A unique identifier for the sound.
     * @param is3D Whether the sound is 3D.
     * @param isLoop Whether the sound should loop.
     * @param volume The volume of the sound (0.0 to 1.0).
     * @param reverbAmount The amount of reverb to apply (0.0 to 1.0).
     * @param filePath The path to the sound file.
     */
    Sound(const SoundDesc& desc, const std::string& uniqueID, ResourceManager* manager);
    Sound(const std::string& uid, ResourceManager* mgr);

    void OnInspectorGUI() override;
    void onCreateLate() override;

    // Getters
    bool is3D() const { return m_is3D; }
    bool isLoop() const { return m_isLoop; }
    float getVolume() const { return m_volume; }
    float getReverbAmount() const { return m_desc.reverbAmount; }
    bool isLoaded() const { return m_loaded; }
    float getX() const { return x; }
    float getY() const { return y; }
    float getZ() const { return z; }

    // Setters
    void setVolume(float newVolume) { m_desc.volume = newVolume; }
    void setReverbAmount(float newReverbAmount) { m_desc.reverbAmount = newReverbAmount; }
    void setLoaded(bool isLoaded) { m_loaded = isLoaded; }
    void set3DCoords(float newX, float newY, float newZ) {
        x = newX;
        y = newY;
        z = newZ;
    }

private:
    SoundDesc m_desc = {};
    bool m_is3D = false;
    bool m_isLoop = false;
    float m_volume = 1.0f;
    float m_reverbAmount = 0.0f;

    bool m_loaded = false;
    float x, y, z;         // 3D coordinates for the sound

    SERIALIZABLE_MEMBERS(m_path, m_is3D, m_isLoop, m_volume, m_reverbAmount)
};
REGISTER_RESOURCE(Sound)

inline void to_json(json& j, SoundPtr const& sound) {
    if (sound)
    {
        j = json{ { "id", sound->getUniqueID() } };
    }
    else {
        j = nullptr;
    }
}

inline void from_json(json const& j, SoundPtr& sound) {
    if (j.contains("id")) sound = ResourceManager::GetInstance().getSound(j["id"].get<string>());
}


#endif // SOUNDINFO_H