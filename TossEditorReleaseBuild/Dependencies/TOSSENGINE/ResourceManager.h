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
class TOSSENGINE_API ResourceManager
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

    ShaderPtr getShader(const std::string& uniqueId);
    MeshPtr getMesh(const std::string& uniqueId);
    TextureCubeMapPtr getCubemapTexture(const std::string& uniqueId);
    TexturePtr getTexture(const std::string& uniqueId);
    MaterialPtr getMaterial(const std::string& uniqueId);

    // Methods to create various resources
    ShaderPtr createShader(const ShaderDesc& desc, const std::string& uniqueID);
    ShaderPtr createComputeShader(const string& computeShaderFilename);
    TextureCubeMapPtr createCubeMapTextureFromFile(const std::vector<std::string>& filepaths);
    Texture2DPtr createTexture2DFromFile(const std::string& filepath, TextureType type = TextureType::Default);
    Texture2DPtr createTexture2D(Texture2DDesc desc, string textureName = "NoTextureName");
    MeshPtr createMeshFromFile(const std::string& filepath);
    HeightMapPtr createHeightMap(HeightMapInfo& _buildInfo);
    SoundPtr createSound(const SoundDesc& desc, const std::string& uniqueID, const std::string& filepath);
    MaterialPtr createMaterial(const string& shaderId, const std::string& uniqueID);

    void deleteTexture(TexturePtr texture);
    void saveResourcesDescs(const std::string& filepath);
    void loadResourceDesc(const std::string& filepath);
    void ClearInstancesFromMeshes();

    bool IsResourceLoaded(const std::string& uniqueId) const;
    std::map<std::string, ResourcePtr>& GetAllResources() { return m_mapResources; }
    void SetSelectedResource(ResourcePtr selectedResource);
    ResourcePtr GetSelectedResource();

protected:
    std::map<std::string, ResourcePtr> m_mapResources; // Map of resources keyed by their unique ids
    ResourcePtr m_selectedResource = nullptr; // for editor use

    std::unordered_map<std::string, ShaderDesc> shaderDescriptions;
    std::vector<std::string> texture2DFilePaths;
    std::unordered_map<std::string, vector<std::string>> cubemapTextureFilePaths;
    std::unordered_map<std::string, std::string> materialDescriptions;

private:
    ResourceManager() = default;
    ~ResourceManager() = default;
};

