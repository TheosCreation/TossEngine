/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : ResourceManager.h
Description : ResourceManager class manages the resources created with the resource class
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include <map>
#include <string>
#include "Utils.h"

/**
 * @class ResourceManager
 * @brief Manages the resources created with the Resource class.
 */
class ResourceManager
{
public:
    // Singleton access method
    static ResourceManager& GetInstance()
    {
        static ResourceManager instance;
        return instance;
    }

    // Delete copy constructor and assignment operator
    ResourceManager(const ResourceManager& _copy) = delete;
    ResourceManager& operator=(const ResourceManager& _copy) = delete;

    // Methods to create various resources
    TextureCubeMapPtr createCubeMapTextureFromFile(const std::vector<std::string>& filepaths);
    Texture2DPtr createTexture2DFromFile(const std::string& filepath, TextureType type = TextureType::Default);
    MeshPtr createMeshFromFile(const std::string& filepath);
    InstancedMeshPtr createInstancedMeshFromFile(const std::string& filepath);
    HeightMapPtr createHeightMap(HeightMapInfo& _buildInfo);

    TexturePtr getSkyboxTexture();

    void ClearInstancesFromMeshes();

protected:
    std::map<std::string, ResourcePtr> m_mapResources; // Map of resources keyed by their file paths
    TextureCubeMapPtr m_textureCubeMap; // Texture pointer for the skybox

private:
    ResourceManager() = default;
    ~ResourceManager() = default;
};