#include "Sound.h"

Sound::Sound(const SoundDesc& desc, const std::string& uniqueID, const char* path, ResourceManager* manager) : Resource(path, uniqueID, manager)
{
	m_desc = desc;
}
