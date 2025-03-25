#include "Sound.h"

Sound::Sound(const SoundDesc& desc, const std::string& filePath, const std::string& uniqueID, ResourceManager* manager) : Resource(filePath, uniqueID, manager)
{
	m_desc = desc;
}