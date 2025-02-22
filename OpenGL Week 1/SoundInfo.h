/// 
/// @file SoundInfo.h
/// @author Ross Hoyt
///
#ifndef SOUNDINFO_H
#define SOUNDINFO_H

#include <string>
#include <map>

class SoundInfo {
public:
    /**
     * Constructor for SoundInfo.
     * @param filePath The path to the sound file.
     * @param uniqueID A unique identifier for the sound.
     * @param is3D Whether the sound is 3D.
     * @param isLoop Whether the sound should loop.
     * @param volume The volume of the sound (0.0 to 1.0).
     * @param reverbAmount The amount of reverb to apply (0.0 to 1.0).
     */
    SoundInfo(const std::string& filePath, const std::string& uniqueID, bool is3D = false, bool isLoop = false, float volume = 1.0f, float reverbAmount = 0.0f)
        : filePath(filePath), uniqueID(uniqueID), m_is3D(is3D), m_isLoop(isLoop), m_volume(volume), m_reverbAmount(reverbAmount), m_loaded(false), x(0.0f), y(0.0f), z(0.0f) {
    }

    // Getters
    const std::string& getFilePath() const { return filePath; }
    const std::string& getUniqueID() const { return uniqueID; }
    bool is3D() const { return m_is3D; }
    bool isLoop() const { return m_isLoop; }
    float getVolume() const { return m_volume; }
    float getReverbAmount() const { return m_reverbAmount; }
    bool isLoaded() const { return m_loaded; }
    float getX() const { return x; }
    float getY() const { return y; }
    float getZ() const { return z; }

    // Setters
    void setVolume(float newVolume) { m_volume = newVolume; }
    void setReverbAmount(float newReverbAmount) { m_reverbAmount = newReverbAmount; }
    void setLoaded(bool isLoaded) { m_loaded = isLoaded; }
    void set3DCoords(float newX, float newY, float newZ) {
        x = newX;
        y = newY;
        z = newZ;
    }

private:
    std::string filePath;  // Path to the sound file
    std::string uniqueID;  // Unique identifier for the sound
    bool m_is3D;             // Whether the sound is 3D
    bool m_isLoop;           // Whether the sound should loop
    float m_volume;          // Volume of the sound (0.0 to 1.0)
    float m_reverbAmount;    // Amount of reverb to apply (0.0 to 1.0)
    bool m_loaded;           // Whether the sound has been loaded
    float x, y, z;         // 3D coordinates for the sound
};

#endif // SOUNDINFO_H