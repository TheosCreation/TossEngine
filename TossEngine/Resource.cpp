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

Resource::Resource(const string& path, const string& uniqueId, ResourceManager* manager) : m_path(path), m_uniqueID(uniqueId)
{
}

Resource::~Resource()
{
}

std::string Resource::getPath()
{
	return m_path;
}

const std::string& Resource::getUniqueID()
{
	return m_uniqueID;
}

void Resource::setUniqueID(const std::string& id)
{
    m_uniqueID = id;
}
