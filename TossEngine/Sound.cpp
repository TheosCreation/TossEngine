#include "Sound.h"
#include "AudioEngine.h"

Sound::Sound(const SoundDesc& desc, const std::string& uniqueID, ResourceManager* manager) : Resource(desc.filepath, uniqueID, manager)
{
	m_desc = desc;
}

Sound::Sound(const std::string& uid, ResourceManager* mgr) : Resource(uid, mgr)
{
}

void Sound::onCreateLate()
{
    AudioEngine::GetInstance().loadSound(*this);
}
