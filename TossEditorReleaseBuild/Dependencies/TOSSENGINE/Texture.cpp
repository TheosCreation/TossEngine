/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Texture.cpp
Description : Texture class handles the loading and management of texture resources.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "Texture.h"

Texture::Texture() : Resource("NoFileID", "NoTextureID", nullptr)
{
}

Texture::Texture(const string& filePath, ResourceManager* manager) : Resource(filePath, filePath, manager)
{
}

void Texture::OnInspectorGUI()
{
    ImGui::Text(("Texture Inspector - ID: " + m_uniqueID).c_str());
    ImGui::Separator();
}

bool Texture::Delete(bool deleteSelf)
{
    return false;
}

Texture::~Texture()
{
}

uint Texture::getId() const
{
    return m_textureId;
}

void Texture::setId(uint id)
{
    m_textureId = id;
}
