/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : ResourceManager.cpp
Description : ResourceManager class manages the resources created with the resource class
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "ResourceManager.h"
#include "InstancedMesh.h"
#include "Mesh.h"
#include "Texture2D.h"
#include "TextureCubeMap.h"
#include "HeightMap.h"
#include "Mesh.h"
#include "Sound.h"
#include "AudioEngine.h"
#include "Game.h"
#include <filesystem>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
TextureCubeMapPtr ResourceManager::createCubeMapTextureFromFile(const std::vector<std::string>& filepaths)
{

    stbi_set_flip_vertically_on_load(false);

    if (filepaths.size() != 6)
    {
        Debug::LogError("Cubemap texture requires exactly 6 images");
        return TextureCubeMapPtr();
    }

    TextureCubeMapDesc desc;
    for (const auto& filepath : filepaths)
    {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            desc.textureData.push_back(data);
            desc.textureSize = { width, height };
            desc.numChannels = nrChannels;
        }
        else
        {
            Debug::LogError("Cubemap texture failed to load at path: " + filepath);
            stbi_image_free(data);
            return TextureCubeMapPtr();
        }
    }

    // Create a cubemap texture using the graphics engine.
    TextureCubeMapPtr textureCubeMapPtr = std::make_shared<TextureCubeMap>(desc, filepaths[0].c_str(), "", this);
    if (!textureCubeMapPtr)
    {
        Debug::LogError("Cubemap texture not generated");
    }

    // Free the image data.
    for (auto data : desc.textureData)
    {
        stbi_image_free(data);
    }

    if (textureCubeMapPtr)
    {
        m_textureCubeMap = textureCubeMapPtr;
        return textureCubeMapPtr;
    }

    return TextureCubeMapPtr();
}

Texture2DPtr ResourceManager::createTexture2DFromFile(const std::string& filepath, TextureType type)
{
    // Check if the resource has already been loaded
    auto it = m_mapResources.find(filepath);
    if (it != m_mapResources.end())
    {
        return std::static_pointer_cast<Texture2D>(it->second);
    }
    stbi_set_flip_vertically_on_load(false);

    // Load the image data using stb_image.
    Texture2DDesc desc;
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);
    if (!data)
    {
        Debug::LogError("Texture failed to load at path: " + filepath);
        return Texture2DPtr();
    }

    desc.textureData = data;
    desc.textureSize = { width, height };
    desc.numChannels = nrChannels;

    // Create a 2D texture using the graphics engine.
    Texture2DPtr texture2DPtr = std::make_shared<Texture2D>(desc, filepath.c_str(), "", this);
    if (!texture2DPtr)
    {
        Debug::LogError("Texture not generated");
    }

    // Free the image data.
    stbi_image_free(data);

    if (texture2DPtr)
    {
        m_mapResources.emplace(filepath, texture2DPtr);
        return texture2DPtr;
    }

    return Texture2DPtr();
}

Texture2DPtr ResourceManager::createTexture2D(Texture2DDesc desc, string textureName)
{
    // Create a 2D texture using the graphics engine.
    Texture2DPtr texture2DPtr = std::make_shared<Texture2D>(desc, "", "", this);
    if (!texture2DPtr)
    {
        Debug::LogError("Texture not generated");
    }

    if (texture2DPtr)
    {
        m_mapResources.emplace(textureName, texture2DPtr);
        return texture2DPtr;
    }

    return Texture2DPtr();
}

MeshPtr ResourceManager::createMeshFromFile(const std::string& filepath)
{
    // Check if the resource has already been loaded
    auto it = m_mapResources.find(filepath);
    if (it != m_mapResources.end())
    {
        return std::static_pointer_cast<Mesh>(it->second);
    }

    MeshPtr meshPtr = std::make_shared<Mesh>(filepath.c_str(), this);
    if (meshPtr)
    {
        m_mapResources.emplace(filepath, meshPtr);
        return meshPtr;
    }
    return MeshPtr();
}

InstancedMeshPtr ResourceManager::createInstancedMeshFromFile(const std::string& filepath)
{
    // Check if the resource has already been loaded
    auto it = m_mapResources.find(filepath);
    if (it != m_mapResources.end())
    {
        return std::static_pointer_cast<InstancedMesh>(it->second);
    }

    InstancedMeshPtr instancedMeshPtr = std::make_shared<InstancedMesh>(filepath.c_str(), this);
    if (instancedMeshPtr)
    {
        m_mapResources.emplace(filepath, instancedMeshPtr);
        return instancedMeshPtr;
    }

    return InstancedMeshPtr();
}

HeightMapPtr ResourceManager::createHeightMap(HeightMapInfo& _buildInfo)
{
    HeightMapDesc desc;
    uint vertexCount = _buildInfo.width * _buildInfo.depth;
    std::vector<unsigned char> heightValue(vertexCount);

    std::ifstream file(_buildInfo.filePath, std::ios_base::binary);
    if (file) {
        file.read((char*)&heightValue[0], (std::streamsize)heightValue.size());
        file.close();
    }
    else {
        Debug::LogError("Error: Height map failed to load: " + _buildInfo.filePath);
        return HeightMapPtr();  // Return null pointer if file reading fails
    }

    // Convert height values from unsigned char to float
    desc.data.resize(vertexCount, 0);
    for (uint i = 0; i < vertexCount; ++i) {
        desc.data[i] = (float)heightValue[i];
    }

    // Create a HeightMap
    HeightMapPtr heightMapPtr = std::make_shared<HeightMap>(desc, _buildInfo, _buildInfo.filePath.c_str(), this);
    if (!heightMapPtr) {
        Debug::LogError("Heightmap not generated");
        return HeightMapPtr();  // Return null pointer if HeightMap creation fails
    }

    // Store and return the created HeightMap
    m_mapResources.emplace(_buildInfo.filePath, heightMapPtr);
    return heightMapPtr;
}

SoundPtr ResourceManager::createSound(const SoundDesc& desc, const std::string& uniqueID, const std::string& filepath)
{
    SoundPtr soundPtr = std::make_shared<Sound>(desc, uniqueID, filepath.c_str(), this);
    AudioEngine::GetInstance().loadSound(soundPtr);
    return soundPtr;
}


TexturePtr ResourceManager::getSkyboxTexture()
{
    return m_textureCubeMap;
}

void ResourceManager::deleteTexture(TexturePtr texture)
{
    for (auto it = m_mapResources.begin(); it != m_mapResources.end(); ++it)
    {
        if (it->second == texture)
        {
            m_mapResources.erase(it); // Remove from map

            if (texture.use_count() == 1) // If it's the last reference
            {
                uint id = texture->getId();
                glDeleteTextures(1, &id); // Delete OpenGL texture
            }

            return;
        }
    }
}

void ResourceManager::ClearInstancesFromMeshes()
{
    // Iterate over all resources in the map
    for (auto& [key, resource] : m_mapResources)
    {
        // Check if the resource is of type InstancedMesh
        InstancedMeshPtr instancedMeshPtr = std::dynamic_pointer_cast<InstancedMesh>(resource);
        if (instancedMeshPtr)
        {
            instancedMeshPtr->clearInstances();
        }
    }
}