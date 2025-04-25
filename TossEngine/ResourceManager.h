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
    void SetSelectedResource(const ResourcePtr& selectedResource);
    ResourcePtr GetSelectedResource();
    ResourcePtr GetResourceByUniqueID(const std::string& id);
    void RenameResource(ResourcePtr resource, const std::string& newId);

    void DeleteResource(const std::string& uniqueId);
    void Reload();
    void CleanUp();


    vector<PrefabPtr> getPrefabs() const;
    void DeletePrefabs();
    void LoadPrefabs();

protected:
    bool hasLoadedResources = false;
    bool hasCreatedResources = false;
    std::map<string, ResourcePtr> m_mapResources; // Map of resources keyed by their unique ids
    std::unordered_map<string, json> m_resourceDataMap;

    std::unordered_map<std::string, std::function<ResourcePtr(const std::string& uniqueId, ResourceManager* mgr)>> resourceFactories;

    std::set<string> m_resourcesToDestroy;
    ResourcePtr m_selectedResource = nullptr; // for editor use
    size_t m_nextAvailableId = 1;
    string m_currentFilePath = "";

private:
    ResourceManager() = default;
    ~ResourceManager() = default;
};

#define REGISTER_RESOURCE(T) \
    static bool _registered_##T = []() { \
        ResourceManager::GetInstance().registerResource<T>(); \
        return true; \
    }();