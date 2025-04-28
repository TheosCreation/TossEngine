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
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "Prefab.h"
#include "AudioEngine.h"
#include <filesystem>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

bool ResourceManager::IsResourceLoaded(const std::string& uniqueId) const
{
    return m_mapResources.find(uniqueId) != m_mapResources.end();
}

ResourcePtr ResourceManager::GetSelectedResource()
{
    return m_selectedResource;
}

ResourcePtr ResourceManager::GetResourceByUniqueID(const std::string& id)
{
    if (m_mapResources.find(id) != m_mapResources.end())
    {
        return m_mapResources[id];
    }
    return nullptr;
}

void ResourceManager::RenameResource(ResourcePtr resource, const std::string& newId)
{
    std::string oldId = resource->getUniqueID();

    // Make sure the old ID exists
    auto it = m_mapResources.find(oldId);
    if (it == m_mapResources.end()) {
        Debug::LogError("RenameResource failed: old ID not found: " + oldId, false);
        return;
    }

    // Prevent overwriting an existing resource
    if (m_mapResources.find(newId) != m_mapResources.end()) {
        Debug::LogError("RenameResource failed: new ID already exists: " + newId, false);
        return;
    }

    // Update the resource ID
    resource->setUniqueID(newId);
    m_mapResources[newId] = resource;
    m_mapResources.erase(it);

}

void ResourceManager::DeleteResource(const std::string& uniqueId)
{
    m_resourcesToDestroy.insert(uniqueId);
}

void ResourceManager::Reload()
{
    m_selectedResource.reset();

    for (auto& [key, resource] : m_mapResources)
    {
        resource->onDestroy();
    }
    m_mapResources.clear();
    loadResourcesFromFile(m_currentFilePath);
}

void ResourceManager::CleanUp()
{
    m_selectedResource.reset();

    for (auto& [key, resource] : m_mapResources)
    {
        resource->onDestroy();
    }
    m_mapResources.clear();

    //we only call cleanup on quit because the resource factories are created on launch
    resourceFactories.clear();
}

void ResourceManager::addResourceModule(const std::type_info& typeInfo)
{
    std::string typeName = getClassName(typeInfo);  // Always get the type name

    void* moduleHandle = nullptr;

#ifdef _WIN32
    // Get module (DLL) handle from the RTTI object address
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery((void*)&typeInfo, &mbi, sizeof(mbi))) {
        moduleHandle = mbi.AllocationBase;
    }
#endif

    m_resourceModules[typeName] = moduleHandle;
}

void ResourceManager::CleanUpModule(void* moduleHandle)
{
    for (auto it = m_resourceModules.begin(); it != m_resourceModules.end(); )
    {
        if (it->second == moduleHandle)
        {
            std::string typeName = it->first;
            Debug::Log("Unregistering component: " + typeName);

            resourceFactories.erase(typeName);
            it = m_resourceModules.erase(it); // remove from modules and move iterator forward
        }
        else
        {
            ++it;
        }
    }
}

vector<PrefabPtr> ResourceManager::getPrefabs() const
{
    vector<PrefabPtr> prefabs;
    for (const auto& pair : m_mapResources)
    {
        if (PrefabPtr prefab = std::dynamic_pointer_cast<Prefab>(pair.second))
        {
            prefabs.push_back(prefab);
        }
    }
    return prefabs;
}

void ResourceManager::DeletePrefabs()
{
    m_resourceDataMap.clear();

    std::vector<std::string> toRemove;
    toRemove.reserve(m_mapResources.size());

    for (auto& [uid, resource] : m_mapResources)
    {
        if (auto prefab = std::dynamic_pointer_cast<Prefab>(resource))
        {
            // Backup its data

            json j = prefab->serialize();
            j["uniqueId"] = uid;

            // Store the full payload
            m_resourceDataMap[uid] = std::move(j);

            // Let the prefab clean up its runtime (e.g. detach from scene)
            prefab->onDestroy();

            // Mark for removal from the live map
            toRemove.push_back(uid);
        }
    }

    for (auto& uid : toRemove)
        m_mapResources.erase(uid);
}

void ResourceManager::LoadPrefabs()
{
    // Re-create each prefab from the JSON we saved
    for (auto& [uid, prefabData] : m_resourceDataMap)
    {
        // This will internally call Prefab::onCreate() and onCreateLate()
        createResourceFromDataToMap("Prefab", prefabData);
    }

    // Clear the backup now that everything is back in m_mapResources
    m_resourceDataMap.clear();
}

void ResourceManager::SetSelectedResource(const ResourcePtr& selectedResource)
{
    m_selectedResource = selectedResource;
}

void ResourceManager::loadResourcesFromFile(const std::string& filepath)
{
    if (hasLoadedResources) return;
    m_currentFilePath = filepath;
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        Debug::LogError("Failed to open file for reading: " + filepath, false);
        return;
    }

    json data;
    file >> data;
    file.close();

    for (const auto& resourceData : data["resources"])
    {
        if (resourceData.contains("type"))
        {
            std::string typeName = resourceData["type"];
            createResourceFromDataToMap(typeName, resourceData);
        }
    }

    for (auto& [uid, resource] : m_mapResources)
    {
        resource->onCreate();
        if (m_resourceDataMap.contains(uid))
        {
            resource->deserialize(m_resourceDataMap[uid]);
        }
        resource->onCreateLate();
    }


    hasLoadedResources = true;
}


ResourcePtr ResourceManager::createResourceFromDataToMap(const std::string& typeName, const json& data)
{
    auto it = resourceFactories.find(typeName);
    if (it == resourceFactories.end()) {
        Debug::LogError("No resource factory for type “" + typeName + "”", false);
        return nullptr;
    }

    if (data.contains("uniqueId"))
    {
        std::string uid = data["uniqueId"];
        auto res = it->second(uid, this);
        if (res) {
            m_mapResources.emplace(uid, res);
            m_resourceDataMap.emplace(uid, data); // for later when we deserialize all resources
        }
        return res;
    }
    else
    {
        Debug::LogError("Resource data loaded has no id");
    }
    return nullptr;
}

ResourcePtr ResourceManager::createResource(const std::string& typeName, const std::string& uid, const json& data)
{
    auto it = resourceFactories.find(typeName);
    if (it == resourceFactories.end()) {
        Debug::LogError("No resource factory for type “" + typeName + "”", false);
        return nullptr;
    }

    auto res = it->second(uid, this);
    if (res) {
        m_mapResources.emplace(uid, res);
        res->onCreate();
        if (data != nullptr) res->deserialize(data);
        res->onCreateLate();
    }
    return res;
}

vector<string> ResourceManager::GetCreatableResourceTypes() const
{
    std::vector<std::string> types;
    types.reserve(resourceFactories.size());
    for (auto const& kv : resourceFactories)
        types.push_back(kv.first);

    std::sort(types.begin(), types.end());
    return types;
}


void ResourceManager::saveResourcesToFile(const std::string& filepath)
{
    json out;
    out["resources"] = nlohmann::json::array();

    for (auto& [uid, resource] : m_mapResources)
    {
        // get the payload for *this* resource
        json payload = resource->serialize();

        // inject type + uniqueId
        payload["type"] = getClassName(typeid(*resource));
        payload["uniqueId"] = uid;

        out["resources"].push_back(std::move(payload));
    }

    // write to disk
    std::ofstream file(filepath);
    if (!file.is_open()) {
        Debug::LogError("Failed to open file for writing: " + filepath, false);
        return;
    }
    file << out.dump(4);
    file.close();

    Debug::Log("Saved Resources to filepath: " + filepath);
}


void ResourceManager::onUpdateInternal()
{
    for (auto& [key, resource] : m_mapResources) {
        if (m_resourcesToDestroy.count(key)) continue;
        if (auto prefabPtr = std::dynamic_pointer_cast<Prefab>(resource)) {
            prefabPtr->onUpdateInternal();
        }
    }
    for (auto const& id : m_resourcesToDestroy) {
        auto it = m_mapResources.find(id);
        if (it != m_mapResources.end()) {
            auto& resource = it->second;
            resource->onDestroy();
            if (m_selectedResource == resource) {
                m_selectedResource = nullptr;
            }
            m_mapResources.erase(it);
            Debug::Log("Deleted resource with ID: " + id);
        }
        else {
            Debug::Log("Attempted to delete non‑existent resource ID: " + id);
        }
    }
    m_resourcesToDestroy.clear();
}