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
#include <set>
#include <string>
#include "Utils.h"
#include "CoroutineTask.h"

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
    MeshPtr getMesh(const std::string& uniqueId, bool createIfNotFound = true);
    TextureCubeMapPtr getTextureCubeMap(const std::string& uniqueId);
    TexturePtr getTexture(const std::string& uniqueId);
    MaterialPtr getMaterial(const std::string& uniqueId);
    PhysicsMaterialPtr getPhysicsMaterial(const std::string& uniqueId);
    SoundPtr getSound(const std::string& uniqueId);
    PrefabPtr getPrefab(const std::string& uniqueId);
    vector<PrefabPtr> getPrefabs() const;

    // Methods to create various resources
    ShaderPtr createShader(const ShaderDesc& desc, const std::string& uniqueID);
    ShaderPtr createComputeShader(const string& computeShaderFilename);
    TextureCubeMapPtr createCubeMapTextureFromFile(const std::vector<std::string>& filepaths, const string& uniqueId);
    Texture2DPtr createTexture2DFromFile(const string& filepath, const string& uniqueId, TextureType type = TextureType::Default);
    Texture2DPtr createTexture2D(Texture2DDesc desc, string textureName = "NoTextureName");
    MeshPtr createMesh(MeshDesc desc, const string& uniqueId);
    HeightMapPtr createHeightMap(HeightMapInfo& _buildInfo);
    SoundPtr createSound(const SoundDesc& desc, const std::string& uniqueID);
    MaterialPtr createMaterial(const MaterialDesc& materialDesc, const std::string& uniqueID);
    PhysicsMaterialPtr createPhysicsMaterial(const PhysicsMaterialDesc& desc, const std::string& uniqueID);
    PhysicsMaterialPtr createPhysicsMaterial(const std::string& uniqueID, const json& data = nullptr);
    PrefabPtr createPrefab(const string& id, const json& data = nullptr);

    void deleteTexture(TexturePtr texture);
    CoroutineTask saveResourcesDescs(const std::string& filepath);
    CoroutineTask loadResourceDesc(const std::string& filepath);
    void ClearInstancesFromMeshes();

    CoroutineTask createResourcesFromDescs();
    void onUpdateInternal();

    bool IsResourceLoaded(const std::string& uniqueId) const;
    std::map<std::string, ResourcePtr>& GetAllResources() { return m_mapResources; }
    void SetSelectedResource(ResourcePtr selectedResource);
    ResourcePtr GetSelectedResource();
    ResourcePtr GetResourceByUniqueID(const std::string& id);
    void RenameResource(ResourcePtr resource, const std::string& newId);

    void DeleteResource(const std::string& uniqueId);

    void DeleteFromSavedData(const std::shared_ptr<Resource>& resource, const std::string& uniqueId);
    void CleanUp();

protected:
    bool hasLoadedResources = false;
    bool hasCreatedResources = false;
    std::map<string, ResourcePtr> m_mapResources; // Map of resources keyed by their unique ids

    std::set<string> m_resourcesToDestroy;
    ResourcePtr m_selectedResource = nullptr; // for editor use
    size_t m_nextAvailableId = 1;
    std::unordered_map<string, ShaderDesc> shaderDescriptions;
    std::unordered_map<string, string> texture2DFilePaths;
    std::unordered_map<string, vector<string>> cubemapTextureFilePaths;
    std::unordered_map<string, MaterialDesc> materialDescs;
    std::unordered_map<string, MeshDesc> meshDescriptions;
    std::unordered_map<string, json> physicsMaterialDescriptions;
    std::unordered_map<string, json> m_prefabDescs;
    std::unordered_map<string, SoundDesc> m_soundDescs;

private:
    ResourceManager() = default;
    ~ResourceManager() = default;
};

