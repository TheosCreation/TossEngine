#include "Sound.h"

Sound::Sound(const SoundDesc& desc, const std::string& uniqueID, ResourceManager* manager) : Resource(desc.filepath, uniqueID, manager)
{
	m_desc = desc;
}