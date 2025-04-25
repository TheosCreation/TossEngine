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

    template<typename T>
    std::shared_ptr<T> get(const std::string& id)
    {
        auto it = m_mapResources.find(id);
        if (!id.empty() && it != m_mapResources.end())
            return std::dynamic_pointer_cast<T>(it->second);
        return nullptr;
    }
    template<typename T>
        void registerResource() {
        // T must have:
        //   T(const std::string& uid, ResourceManager* mgr);
        std::string typeName = getClassName(typeid(T));
        resourceFactories[typeName] =
            [](const std::string& uid, ResourceManager* mgr) -> ResourcePtr {
            return std::make_shared<T>(uid, mgr);
            };
    }
    CoroutineTask loadResourcesFromFile(const std::string& filepath);
    CoroutineTask saveResourcesToFile(const std::string& filepath);
    ResourcePtr createResourceFromData(const std::string& typeName, const json& data);
    ResourcePtr createResource(const std::string& typeName, const std::string& uid);

    vector<string> GetCreatableResourceTypes() const;

    void onUpdateInternal();

    bool IsResourceLoaded(const std::string& uniqueId) const;
    std::map<std::string, ResourcePtr>& GetAllResources() { return m_mapResources; }
    void SetSelectedResource(ResourcePtr selectedResource);
    ResourcePtr GetSelectedResource();
    ResourcePtr GetResourceByUniqueID(const std::string& id);
    void RenameResource(ResourcePtr resource, const std::string& newId);

    void DeleteResource(const std::string& uniqueId);
    void Reload();
    void CleanUp();


    void LoadPrefabs();
    void DeletePrefabs();

    //old functions
    // replace the getters with a typed function
    ShaderPtr getShader(const std::string& uniqueId);
    MeshPtr getMesh(const std::string& uniqueId, bool createIfNotFound = true);
    TextureCubeMapPtr getTextureCubeMap(const std::string& uniqueId);
    Texture2DPtr getTexture2D(const std::string& uniqueId);
    MaterialPtr getMaterial(const std::string& uniqueId);
    PhysicsMaterialPtr getPhysicsMaterial(const std::string& uniqueId);
    SoundPtr getSound(const std::string& uniqueId);
    PrefabPtr getPrefab(const std::string& uniqueId);
    FontPtr getFont(const std::string& uniqueId);
    vector<PrefabPtr> getPrefabs() const;

    // Methods to create various resources
    ShaderPtr createShader(const ShaderDesc& desc, const std::string& uniqueID);
    ShaderPtr createComputeShader(const string& computeShaderFilename);
    TextureCubeMapPtr createCubeMapTextureFromFile(const std::vector<std::string>& filepaths, const string& uniqueId);
    Texture2DPtr createTexture2DFromFile(const string& filepath, const string& uniqueId, TextureType type = TextureType::Default);
    Texture2DPtr createTexture2D(Texture2DDesc desc, string textureName = "NoTextureName");
    FontPtr createFont(const string& uniqueId, const string& filepath);
    MeshPtr createMesh(MeshDesc desc, const string& uniqueId);
    HeightMapPtr createHeightMap(HeightMapInfo& _buildInfo);
    SoundPtr createSound(const SoundDesc& desc, const std::string& uniqueID);
    MaterialPtr createMaterial(const MaterialDesc& materialDesc, const std::string& uniqueID);
    PhysicsMaterialPtr createPhysicsMaterial(const PhysicsMaterialDesc& desc, const std::string& uniqueID);
    PhysicsMaterialPtr createPhysicsMaterial(const std::string& uniqueID, const json& data = nullptr);
    PrefabPtr createPrefab(const string& id, const json& data = nullptr);

    void deleteTexture(Texture2DPtr texture);
    CoroutineTask saveResourcesDescs(const std::string& filepath);
    CoroutineTask loadResourceDesc(const std::string& filepath);
    void ClearInstancesFromMeshes();



    CoroutineTask createResourcesFromDescs();

    void DeleteFromSavedData(const std::shared_ptr<Resource>& resource, const std::string& uniqueId);

protected:
    bool hasLoadedResources = false;
    bool hasCreatedResources = false;
    std::map<string, ResourcePtr> m_mapResources; // Map of resources keyed by their unique ids
    std::unordered_map<string, json> m_resourceDataMap;

    std::unordered_map<std::string, std::function<ResourcePtr(const std::string& uniqueId, ResourceManager* mgr)>> resourceFactories;

    std::set<string> m_resourcesToDestroy;
    ResourcePtr m_selectedResource = nullptr; // for editor use
    size_t m_nextAvailableId = 1;

    //old
    std::unordered_map<string, ShaderDesc> shaderDescriptions;
    std::unordered_map<string, string> texture2DFilePaths;
    std::unordered_map<string, string> m_fontTtfFilepaths;
    std::unordered_map<string, vector<string>> cubemapTextureFilePaths;
    std::unordered_map<string, MaterialDesc> materialDescs;
    std::unordered_map<string, MeshDesc> meshDescriptions;
    std::unordered_map<string, json> physicsMaterialDescriptions;
    std::unordered_map<string, json> m_prefabDescs;
    std::unordered_map<string, SoundDesc> m_soundDescs;
    //
private:
    ResourceManager() = default;
    ~ResourceManager() = default;
};

#define REGISTER_RESOURCE(T) \
    static bool _registered_##T = []() { \
        ResourceManager::GetInstance().registerResource<T>(); \
        return true; \
    }();