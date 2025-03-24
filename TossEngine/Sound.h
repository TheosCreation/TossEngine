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
    Sound(const SoundDesc& desc, const std::string& filePath, const std::string& uniqueID, ResourceManager* manager);

    // Getters
    bool is3D() const { return m_desc.is3D; }
    bool isLoop() const { return m_desc.isLoop; }
    float getVolume() const { return m_desc.volume; }
    float getReverbAmount() const { return m_desc.reverbAmount; }
    bool isLoaded() { return m_loaded; }
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
    bool m_loaded = false;           // Whether the sound has been loaded
    float x, y, z;         // 3D coordinates for the sound
};

#endif // SOUNDINFO_H