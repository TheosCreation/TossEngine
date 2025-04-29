/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : ResourceManager.h
Description : ResourceManager class manages the resources created with the Resource class.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include "Utils.h"
#include "CoroutineTask.h"

/**
 * @class ResourceManager
 * @brief Central manager for all loaded resources in the TossEngine (textures, meshes, materials, prefabs, etc).
 */
class TOSSENGINE_API ResourceManager
{
public:
    /**
     * @brief Access the singleton instance of the ResourceManager.
     * @return A reference to the ResourceManager instance.
     */
    static ResourceManager& GetInstance()
    {
        static ResourceManager instance;
        return instance;
    }

    // Delete copy constructor and assignment operator.
    ResourceManager(const ResourceManager& other) = delete;
    ResourceManager& operator=(const ResourceManager& other) = delete;

    /**
     * @brief Retrieves a resource by unique ID and casts it to the specified type.
     * @tparam T The resource type to cast to.
     * @param id Unique identifier of the resource.
     * @return A shared pointer to the resource if found and of correct type, nullptr otherwise.
     */
    template<typename T>
    std::shared_ptr<T> get(const std::string& id)
    {
        auto it = m_mapResources.find(id);
        if (!id.empty() && it != m_mapResources.end())
            return std::dynamic_pointer_cast<T>(it->second);
        return nullptr;
    }

    /**
     * @brief Registers a new resource type to the manager.
     * @tparam T The resource class type.
     */
    template<typename T>
    void registerResource()
    {
        std::string typeName = getClassName(typeid(T));
        if (resourceFactories.contains(typeName))
            return; // Already registered

        addResourceModule(typeid(T));

        resourceFactories[typeName] = [](const std::string& uid, ResourceManager* mgr) -> ResourcePtr {
            return std::make_shared<T>(uid, mgr);
            };
    }

    void loadResourcesFromFile(const std::string& filepath);
    void saveResourcesToFile(const std::string& filepath);

    ResourcePtr createResource(const std::string& typeName, const std::string& uid, const json& data = nullptr);

    vector<string> GetCreatableResourceTypes() const;

    void onUpdateInternal();

    bool IsResourceLoaded(const std::string& uniqueId) const;
    std::map<std::string, ResourcePtr>& GetAllResources();

    void SetSelectedResource(const ResourcePtr& selectedResource);
    ResourcePtr GetSelectedResource();
    ResourcePtr GetResourceByUniqueID(const std::string& id);

    void RenameResource(ResourcePtr resource, const std::string& newId);
    void DeleteResource(const std::string& uniqueId);
    void Reload();
    void CleanUp();

    void addResourceModule(const std::type_info& typeInfo);
    void CleanUpModule(void* moduleHandle);

    vector<PrefabPtr> getPrefabs() const;
    void DeletePrefabs();
    void LoadPrefabs();

protected:
    bool hasLoadedResources = false;    //!< True if resources were loaded from disk.
    bool hasCreatedResources = false;   //!< True if any resources have been created at runtime.

    std::map<string, ResourcePtr> m_mapResources; //!< Map of all loaded resources.
    std::unordered_map<string, json> m_resourceDataMap; //!< Serialized backup of resource data.
    std::unordered_map<string, json> m_prefabBackupData; //!< Serialized prefab-specific backup.

    std::unordered_map<std::string, std::function<ResourcePtr(const std::string&, ResourceManager*)>> resourceFactories; //!< Factories for creating resource instances.
    std::unordered_map<std::string, void*> m_resourceModules; //!< Loaded modules for different resource types.

    std::set<string> m_resourcesToDestroy; //!< Resources scheduled for delayed destruction.
    ResourcePtr m_selectedResource = nullptr; //!< Resource currently selected in the editor (for inspector).

    size_t m_nextAvailableId = 1; //!< Counter for generating new unique IDs.
    std::string m_currentFilePath = ""; //!< Last file path used for saving/loading resources.

private:
    /**
     * @brief Private constructor to enforce singleton usage.
     */
    ResourceManager() = default;
    ~ResourceManager() = default;

    ResourcePtr createResourceFromDataToMap(const std::string& typeName, const json& data);
};

/**
 * @brief Macro to automatically register a resource type during static initialization.
 */
#define REGISTER_RESOURCE(T) \
    static bool _registered_##T = []() { \
        ResourceManager::GetInstance().registerResource<T>(); \
        return true; \
    }();
