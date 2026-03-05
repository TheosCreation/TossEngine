/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Resource.cpp
Description : Resource class represents a generic resource such as a file, image or texture
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "Resource.h"

Resource::Resource(const string& path, const string& uniqueId, ResourceManager* manager) : m_path(path), m_uniqueID(uniqueId), m_resourceManager(manager)
{
}

Resource::Resource(const std::string& uid, ResourceManager* mgr) : m_uniqueID(uid), m_resourceManager(mgr)
{
}

void Resource::onCreateLate()
{
    // if we have overriden the path and made it custom then we load that file and deserialize properly
    if (!m_path.empty() && GetAssetSaveExtension() != ".meta")
    {
        json j;
        std::ifstream file(m_path);
        if (file.is_open())
        {
            try
            {
                file >> j;
                deserialize(j);
            }
            catch (...)
            {
            }
        }
    }
}

std::string Resource::getPath()
{
	return m_path;
}

void Resource::setPath(const std::string& filepath)
{
    m_path = filepath;
}

const std::string& Resource::getUniqueID()
{
	return m_uniqueID;
}

void Resource::setUniqueID(const std::string& id)
{
    m_uniqueID = id;
}
