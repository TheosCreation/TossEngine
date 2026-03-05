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
using std::filesystem::path;
using std::filesystem::directory_options;
using std::filesystem::recursive_directory_iterator;

#ifdef _WIN32
#include <windows.h>
#endif

bool ResourceManager::IsResourceLoaded(const std::string& uniqueId) const
{
    return m_mapResources.find(uniqueId) != m_mapResources.end();
}

std::map<std::string, ResourcePtr>& ResourceManager::GetAllResources()
{
    return m_mapResources;
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

    m_revision += 1;
}

void ResourceManager::DeleteResource(const std::string& uniqueId)
{
    if (IsResourceMandatory(uniqueId))
    {
        Debug::LogWarning("Cannot delete mandatory engine resource: " + uniqueId);
        return;
    }

    m_resourcesToDestroy.insert(uniqueId);
}

void ResourceManager::MarkResourceAsMandatory(const std::string& uniqueId)
{
    if (!uniqueId.empty())
    {
        m_mandatoryResources.insert(uniqueId);
    }
}

bool ResourceManager::IsResourceMandatory(const std::string& uniqueId) const
{
    return m_mandatoryResources.find(uniqueId) != m_mandatoryResources.end();
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
    m_prefabBackupData.clear();

    std::vector<std::string> toRemove;
    toRemove.reserve(m_mapResources.size());

    for (auto& [uid, resource] : m_mapResources)
    {
        if (auto prefab = std::dynamic_pointer_cast<Prefab>(resource))
        {
            // Backup its data
            json j = prefab->serialize();
            j["uniqueId"] = uid;

            m_prefabBackupData[uid] = std::move(j);

            prefab->onDestroy();
            toRemove.push_back(uid);
        }
    }

    for (auto& uid : toRemove)
        m_mapResources.erase(uid);
}

void ResourceManager::LoadPrefabs()
{
    for (auto& [uid, prefabData] : m_prefabBackupData)
    {
        createResourceFromDataToMap("Prefab", prefabData);
    }

    m_prefabBackupData.clear();
}

void ResourceManager::LoadAssetsFromFolder(const std::string& assetsRoot)
{
    path root = path(assetsRoot);
    if (!exists(root))
    {
        return;
    }

    for (recursive_directory_iterator it(root, directory_options::skip_permission_denied), end; it != end; ++it)
    {
        if (!it->is_regular_file())
        {
            continue;
        }

        path p = it->path();

        bool isMetaFile = false;
        path assetPath = p;

        if (toLower(p.extension().string()) == ".meta")
        {
            isMetaFile = true;
            assetPath = p;
            assetPath.replace_extension(); // removes ".meta" only
            // If the real asset exists, we will process it and load this meta as a sidecar there.
            if (std::filesystem::exists(assetPath))
            {
                continue;
            }
        }

        const std::string ext = toLower(assetPath.extension().string());
        const std::string type = GuessTypeFromExt(ext);
        if (type.empty())
        {
            continue;
        }

        std::string rel = assetPath.lexically_relative(root).generic_string();

        AssetImportRec rec{};
        if (!ShouldImport(p.string(), type, rec))
        {
            continue;
        }

        // ImportOne should take the file we read from:
        // - normal file: read from assetPath (== p)
        // - meta-only asset: read from p (the meta file), but UID matches assetPath
        ImportOne(p.string(), rel, type);

        ResourcePtr resource = GetResourceByUniqueID(rel);
        if (resource)
        {
            // If we imported from a real asset file, apply sidecar meta.
            // If we imported from meta-only, p is the meta already, so this would be duplicate; skip.
            if (!isMetaFile)
            {
                std::string metaPath = BuildMetaPath(p.string());
                json metaJson;

                if (std::filesystem::exists(metaPath))
                {
                    if (TryReadJsonFile(metaPath, metaJson))
                    {
                        resource->deserialize(metaJson);
                        m_resourceDataMap[rel] = metaJson;
                    }
                    else
                    {
                        Debug::LogWarning("Failed to parse meta: " + metaPath);
                    }
                }
            }
        }

        rec.uid = rel;
        rec.type = type;
        rec.timestamp = ToTicks(last_write_time(p));
        m_importCache[p.string()] = rec;
    }
}

void ResourceManager::SaveImportCache(const std::string& pathJson)
{
    std::filesystem::create_directories(std::filesystem::path(pathJson).parent_path());
    json j;
    for (auto const& [abs, rec] : m_importCache) {
        j[abs] = { {"uid", rec.uid}, {"type", rec.type}, {"ts", rec.timestamp} };
    }
    std::ofstream f(pathJson);
    if (!f.is_open()) return;
    f << j.dump(2);
}

void ResourceManager::LoadImportCache(const std::string& pathJson)
{
    m_importCache.clear();
    std::ifstream f(pathJson);
    if (!f.is_open()) return;
    json j; f >> j; f.close();
    for (auto& [k, v] : j.items()) {
        AssetImportRec r;
        r.uid = v.value("uid", "");
        r.type = v.value("type", "");
        r.timestamp = v.value("ts", 0ull);
        m_importCache[k] = r;
    }
}

void ResourceManager::SaveResources()
{
    for (auto& [uid, resource] : m_mapResources)
    {
        if (!resource || IsResourceMandatory(uid)) // Dont serialize the resource if the resource is null or is internal resource
        {
            continue;
        }

        json payload = resource->serialize();
        payload["type"] = getClassName(typeid(*resource));
        payload["uniqueId"] = uid;

        std::string basePath = resource->getPath();
        std::string ext = resource->GetAssetSaveExtension();
        std::string savePath = BuildAssetSavePath(basePath, ext);

        if (savePath.empty())
        {
            continue;
        }

        std::filesystem::create_directories(std::filesystem::path(savePath).parent_path());

        std::ofstream file(savePath);
        if (!file.is_open())
        {
            Debug::LogError("Failed to open file for writing: " + savePath, false);
            continue;
        }

        file << payload.dump(4);
        file.close();
    }
}

uint64_t ResourceManager::GetRevision() const
{
    return m_revision;
}

bool ResourceManager::ShouldImport(const std::string& absPath, const std::string& type, AssetImportRec& out)
{
    auto it = m_importCache.find(absPath);
    const auto nowTicks = ToTicks(std::filesystem::last_write_time(absPath));
    if (it == m_importCache.end()) return true;                    // never seen
    if (it->second.type != type) return true;                      // type changed
    if (it->second.timestamp != nowTicks) return true;             // modified
    out = it->second;
    return false;
}

void ResourceManager::ImportOne(const std::string& absPath, const std::string& relUID, const std::string& type)
{
    ResourcePtr res = GetResourceByUniqueID(relUID);
    if (!res)
    {
        res = createResource(type, relUID);
        if (!res)
        {
            Debug::LogError("Import failed for " + relUID, false);
            return;
        }
    }

    res->setPath(absPath);

    json payload;
    payload["m_path"] = absPath;

    auto fn = m_resourceDataMap.find(relUID);
    if (fn != m_resourceDataMap.end())
    {
        fn->second["m_path"] = absPath;
    }
    else
    {
        m_resourceDataMap.emplace(relUID, payload);
    }
}

std::string ResourceManager::GuessTypeFromExt(const std::string& ext)
{
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp") { return "Texture2D"; }
    if (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb")                   { return "Mesh"; }
    if (ext == ".mat" || ext == ".material")                                                 { return "Material"; }
    if (ext == ".prefab")                                                                    { return "Prefab"; }
    if (ext == ".wav" || ext == ".mp3" || ext == ".ogg")                                     { return "Sound"; }
    if (ext == ".shaderprog")                                                                { return "Shader"; }

    return {};
}

std::string ResourceManager::GetExtensionForType(const std::string& typeName)
{
    if (typeName == "Shader")
    {
        return ".shaderprog";
    }
    if (typeName == "Texture2D")
    {
        return ".texture2d";
    }
    if (typeName == "Material")
    {
        return ".material";
    }
    return ".asset";
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
        if (!resourceData.contains("type"))
            continue;

        // if the JSON has a file path, make sure it still exists
        if (resourceData.contains("m_path") && resourceData["m_path"].is_string())
        {
            std::string resourcePath = resourceData["m_path"].get<std::string>();

            if (!resourcePath.empty())
            {
                if (!std::filesystem::exists(resourcePath))
                {
                    Debug::Log("Skipped resource, missing file: " + resourcePath);
                    continue;
                }
                Debug::Log("Loaded resource: " + resourcePath);
            }
        }

        std::string typeName = resourceData["type"];
        createResourceFromDataToMap(typeName, resourceData);
    }

    //Above is old stuff
    const std::string assetsRoot = "Assets";
    //LoadImportCache("Library/importCache.json");            // optional cache location
    LoadAssetsFromFolder(assetsRoot);
    SaveImportCache("Library/importCache.json");

    for (auto& [uid, resource] : m_mapResources)
    {
        if (IsResourceMandatory(uid)) continue;
        
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
        std::string uid = data["uniqueId"].get<std::string>();
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

ResourcePtr ResourceManager::createInternalResource(const std::string& typeName, const std::string& uid, const json& data)
{
    ResourcePtr resource = createResource(typeName, uid, data);
    MarkResourceAsMandatory(uid);
    return resource;
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
        m_revision += 1;
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
        if (IsResourceMandatory(uid))
        {
            continue;
        }
        if (!resource)
        {
            continue;
        }

        std::string resourcePath = resource->getPath();
        if (IsInAssetsFolder(resourcePath))
        {
            continue;
        }

        json payload = resource->serialize();
        payload["type"] = getClassName(typeid(*resource));
        payload["uniqueId"] = uid;

        out["resources"].push_back(std::move(payload));
    }

    std::ofstream file(filepath);
    if (!file.is_open())
    {
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