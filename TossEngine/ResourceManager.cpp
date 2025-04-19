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
#include "Texture2D.h"
#include "TextureCubeMap.h"
#include "HeightMap.h"
#include "Material.h"
#include "PhysicsMaterial.h"
#include "Sound.h"
#include "Prefab.h"
#include "AudioEngine.h"
#include <filesystem>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

ShaderPtr ResourceManager::getShader(const std::string& uniqueId)
{
    auto it = m_mapResources.find(uniqueId);
    if (it != m_mapResources.end())
        return std::static_pointer_cast<Shader>(it->second);

    auto it2 = shaderDescriptions.find(uniqueId);
    if (it2 != shaderDescriptions.end())
        return createShader(it2->second, uniqueId);

    return ShaderPtr(); // Return empty if not found
}

MeshPtr ResourceManager::getMesh(const std::string& uniqueId, bool createIfNotFound)
{
    auto it = m_mapResources.find(uniqueId);
    if (it != m_mapResources.end())
        return std::static_pointer_cast<Mesh>(it->second);

    if (createIfNotFound)
    {
        auto it2 = meshDescriptions.find(uniqueId);
        if (it2 != meshDescriptions.end())

            return createMesh(it2->second, uniqueId);
    }
    return nullptr;
}

TextureCubeMapPtr ResourceManager::getTextureCubeMap(const std::string& uniqueId)
{
    auto it = m_mapResources.find(uniqueId);
    if (it != m_mapResources.end())
        return std::static_pointer_cast<TextureCubeMap>(it->second);

    auto it2 = cubemapTextureFilePaths.find(uniqueId);
    if (it2 != cubemapTextureFilePaths.end())
        return createCubeMapTextureFromFile(it2->second, uniqueId);
}

TexturePtr ResourceManager::getTexture(const std::string& uniqueId)
{
    // Try finding the texture in the resource map first
    auto it = m_mapResources.find(uniqueId);
    if (it != m_mapResources.end())
        return std::static_pointer_cast<Texture>(it->second);

    // Look up in the file path map
    auto it2 = texture2DFilePaths.find(uniqueId);
    if (it2 != texture2DFilePaths.end())
        return createTexture2DFromFile(it2->second, uniqueId);

    return TexturePtr(); // Not found anywhere
}

MaterialPtr ResourceManager::getMaterial(const std::string& uniqueId)
{
    auto it = m_mapResources.find(uniqueId);
    if (it != m_mapResources.end())
        return std::static_pointer_cast<Material>(it->second);

    auto it2 = materialDescs.find(uniqueId);
    if (it2 != materialDescs.end())
        return createMaterial(it2->second, uniqueId);

    return MaterialPtr();
}

PhysicsMaterialPtr ResourceManager::getPhysicsMaterial(const std::string& uniqueId)
{
    auto it = m_mapResources.find(uniqueId);
    if (it != m_mapResources.end())
        return std::static_pointer_cast<PhysicsMaterial>(it->second);

    auto it2 = physicsMaterialDescriptions.find(uniqueId);
    if (it2 != physicsMaterialDescriptions.end())
        return createPhysicsMaterial(uniqueId, it2->second);

    return PhysicsMaterialPtr();
}

SoundPtr ResourceManager::getSound(const std::string& uniqueId)
{
    auto it = m_mapResources.find(uniqueId);
    if (it != m_mapResources.end())
        return std::static_pointer_cast<Sound>(it->second);

    auto it2 = m_soundDescs.find(uniqueId);
    if (it2 != m_soundDescs.end())
        return createSound(it2->second, uniqueId);

    return SoundPtr();
}

PrefabPtr ResourceManager::getPrefab(const std::string& uniqueId)
{
    auto it = m_mapResources.find(uniqueId);
    if (it != m_mapResources.end())
        return std::static_pointer_cast<Prefab>(it->second);

    auto it2 = m_prefabDescs.find(uniqueId);
    if (it2 != m_prefabDescs.end())
        return createPrefab(uniqueId, it2->second);

    return PrefabPtr();
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

ShaderPtr ResourceManager::createShader(const ShaderDesc& desc, const std::string& uniqueID)
{
    ShaderPtr shader = std::make_shared<Shader>(desc, uniqueID, this);
    m_mapResources.emplace(uniqueID, shader);
    if (shaderDescriptions.find(uniqueID) == shaderDescriptions.end())
    {
        shaderDescriptions.emplace(uniqueID, desc);
    }
    return shader;
}

ShaderPtr ResourceManager::createComputeShader(const string& computeShaderFilename)
{
    ShaderPtr shader = std::make_shared<Shader>(computeShaderFilename, this);
    m_mapResources.emplace(computeShaderFilename, shader);

    return shader;
}

TextureCubeMapPtr ResourceManager::createCubeMapTextureFromFile(const std::vector<std::string>& filepaths, const string& uniqueId)
{
    stbi_set_flip_vertically_on_load(false);

    if (filepaths.size() != 6)
    {
        Debug::LogError("Cubemap texture requires exactly 6 images", false);
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
            Debug::LogError("Cubemap texture failed to load at path: " + filepath, false);
            stbi_image_free(data);
            return TextureCubeMapPtr();
        }
    }

    // Create a cubemap texture using the graphics engine.
    TextureCubeMapPtr textureCubeMapPtr = std::make_shared<TextureCubeMap>(desc, uniqueId, this);
    if (!textureCubeMapPtr)
    {
        Debug::LogError("Cubemap texture not generated", false);
    }

    // Free the image data.
    for (auto data : desc.textureData)
    {
        stbi_image_free(data);
    }

    if (textureCubeMapPtr)
    {
        m_mapResources.emplace(uniqueId, textureCubeMapPtr);
        if (cubemapTextureFilePaths.find(uniqueId) == cubemapTextureFilePaths.end())
        {
            cubemapTextureFilePaths.emplace(uniqueId, filepaths);
        }
        return textureCubeMapPtr;
    }

    return TextureCubeMapPtr();
}

Texture2DPtr ResourceManager::createTexture2DFromFile(const std::string& filepath, const string& uniqueId, TextureType type)
{
    // Check if the resource has already been loaded
    auto it = m_mapResources.find(uniqueId);
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
        Debug::LogError("Texture failed to load at path: " + filepath, false);
        return Texture2DPtr();
    }

    desc.textureData = data;
    desc.textureSize = { width, height };
    desc.numChannels = nrChannels;

    // Create a 2D texture using the graphics engine.
    Texture2DPtr texture2DPtr = std::make_shared<Texture2D>(desc, filepath, uniqueId, this);
    if (!texture2DPtr)
    {
        Debug::LogError("Texture not generated", false);
    }

    // Free the image data.
    stbi_image_free(data);

    if (texture2DPtr)
    {
        if (texture2DFilePaths.find(uniqueId) == texture2DFilePaths.end())
        {
            texture2DFilePaths.emplace(uniqueId, filepath);
        }

        m_mapResources.emplace(filepath, texture2DPtr);
        return texture2DPtr;
    }

    return Texture2DPtr();
}

Texture2DPtr ResourceManager::createTexture2D(Texture2DDesc desc, string textureName)
{
    // Create a 2D texture using the graphics engine.
    Texture2DPtr texture2DPtr = std::make_shared<Texture2D>(desc, textureName, textureName, this);
    if (!texture2DPtr)
    {
        Debug::LogError("Texture not generated", false);
    }

    if (texture2DPtr)
    {
        m_mapResources.emplace(textureName, texture2DPtr);
        return texture2DPtr;
    }

    return Texture2DPtr();
}

MeshPtr ResourceManager::createMesh(MeshDesc desc, const string& uniqueId)
{
    // Check if the resource has already been loaded
    auto it = m_mapResources.find(uniqueId);
    if (it != m_mapResources.end())
    {
        return std::static_pointer_cast<Mesh>(it->second);
    }

    MeshPtr meshPtr = std::make_shared<Mesh>(desc, uniqueId, this);
    if (meshPtr)
    {
        if (meshDescriptions.find(uniqueId) == meshDescriptions.end())
        {
            meshDescriptions.emplace(uniqueId, desc);
        }

        m_mapResources.emplace(uniqueId, meshPtr);
        return meshPtr;
    }
    return MeshPtr();
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
        Debug::LogError("Error: Height map failed to load: " + _buildInfo.filePath, false);
        return HeightMapPtr();  // Return null pointer if file reading fails
    }

    // Convert height values from unsigned char to float
    desc.data.resize(vertexCount, 0);
    for (uint i = 0; i < vertexCount; ++i) {
        desc.data[i] = (float)heightValue[i];
    }

    // Create a HeightMap
    HeightMapPtr heightMapPtr = std::make_shared<HeightMap>(desc, _buildInfo, _buildInfo.filePath, this);
    if (!heightMapPtr) {
        Debug::LogError("Heightmap not generated", false);
        return HeightMapPtr();  // Return null pointer if HeightMap creation fails
    }

    // Store and return the created HeightMap
    m_mapResources.emplace(_buildInfo.filePath, heightMapPtr);
    return heightMapPtr;
}

SoundPtr ResourceManager::createSound(const SoundDesc& desc, const std::string& uniqueID)
{
    if (SoundPtr soundPtr = std::make_shared<Sound>(desc, uniqueID, this))
    {
        AudioEngine::GetInstance().loadSound(soundPtr);
        m_soundDescs.emplace(uniqueID, desc);
        m_mapResources.emplace(uniqueID, soundPtr);
        return soundPtr;
    }

    return SoundPtr();
}

MaterialPtr ResourceManager::createMaterial(const MaterialDesc& materialDesc, const std::string& uniqueID)
{
    ShaderPtr shader = getShader(materialDesc.shaderId);
    MaterialPtr materialPtr = std::make_shared<Material>(shader, uniqueID, this);
    if (!materialDesc.serializedData.empty()) materialPtr->deserialize(materialDesc.serializedData);
    materialDescs.emplace(uniqueID, materialDesc);
    m_mapResources.emplace(uniqueID, materialPtr);
    return materialPtr;
}

PhysicsMaterialPtr ResourceManager::createPhysicsMaterial(const PhysicsMaterialDesc& desc, const std::string& uniqueID)
{
    PhysicsMaterialPtr physicsMaterial = std::make_shared<PhysicsMaterial>(desc, uniqueID, this);
    physicsMaterialDescriptions.emplace(uniqueID, physicsMaterial->serialize());
    m_mapResources.emplace(uniqueID, physicsMaterial);
    return physicsMaterial;
}

PhysicsMaterialPtr ResourceManager::createPhysicsMaterial(const std::string& uniqueID, const json& data)
{
    PhysicsMaterialPtr physicsMaterial = std::make_shared<PhysicsMaterial>(PhysicsMaterialDesc(), uniqueID, this);
    if (!data.empty())
    {
        physicsMaterial->deserialize(data);
    }
    m_mapResources.emplace(uniqueID, physicsMaterial);
    return physicsMaterial;
}

PrefabPtr ResourceManager::createPrefab(const string& id, const json& data)
{
    // Create a 2D texture using the graphics engine.
    PrefabPtr prefabPtr = std::make_shared<Prefab>(id, this);
    m_mapResources.emplace(id, prefabPtr);
    prefabPtr->name = id;

    size_t newId = m_nextAvailableId++;
    prefabPtr->setId(newId);
    if (!data.empty())
    {
        prefabPtr->deserialize(data);
    }
    prefabPtr->onCreate();
    prefabPtr->onCreateLate();
    m_prefabDescs.emplace(id, data);
    return prefabPtr;
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

CoroutineTask ResourceManager::saveResourcesDescs(const std::string& filepath)
{
    json data;

    // Serialize shader descriptions properly
    for (auto& [key, shaderDesc] : shaderDescriptions)
    {
        data["shaders"][key] = {
            {"vertexShader", shaderDesc.vertexShaderFilePath},
            {"fragmentShader", shaderDesc.fragmentShaderFilePath}
        };
    }
    
    // Serialize physics descriptions properly

    for (auto& [key, physicsDesc] : physicsMaterialDescriptions)
    {
        PhysicsMaterialPtr material = getPhysicsMaterial(key);
        data["PhysicMaterials"][key] = material->serialize();
    }

    for (auto& [key, meshDesc] : meshDescriptions)
    {
        json instancesJson = json::array();

        // Check if the mesh is available in current resources
        auto mesh = getMesh(key, false);
        if (mesh)
        {
            for (const auto& transform : mesh->getInstanceTransforms())
            {
                instancesJson.push_back(transform.serialize());
            }
        }
        else
        {
            // Fallback to saved transforms if mesh not loaded
            for (const auto& transform : meshDesc.instanceTransforms)
            {
                instancesJson.push_back(transform.serialize());
            }
        }

        data["meshes"][key] = {
            {"filepath", meshDesc.filePath},
            {"instances", instancesJson}
        };
    }


    // Serialize prefab data correctly
    for (auto& [key, desc] : m_prefabDescs)
    {
        PrefabPtr prefab = getPrefab(key);
        data["prefabs"][key] = prefab->serialize();
    }

    // Serialize material descriptions correctly
    for (auto& [key, shaderId] : materialDescs)
    {
        MaterialPtr material = getMaterial(key);
        data["materials"][key] = material->serialize();
    }

    for (auto& [key, soundDesc] : m_soundDescs)
    {
        data["sounds"][key] = {
            {"filepath", soundDesc.filepath}
        };
    }

    // Serialize texture2D file paths
    for (auto& [key, texture2DFilePath] : texture2DFilePaths)
    {
        data["texture2Ds"][key] = texture2DFilePath;
    }

    // Serialize cubemap file paths
    for (auto& [key, cubemapFilePaths] : cubemapTextureFilePaths)
    {
        data["cubemaps"][key] = cubemapFilePaths; // Store the full vector
    }

    // Write to file
    std::ofstream file(filepath);
    if (!file.is_open())
    {
        Debug::LogError("Failed to open file for writing: " + filepath, false);
        co_return;
    }

    file << data.dump(4); // Pretty-print JSON with 4-space indentation
    file.close();

    Debug::Log("Saved Resources to filepath: " + filepath);

    co_return;
}

CoroutineTask ResourceManager::loadResourceDesc(const std::string& filepath)
{
    if (hasLoadedResources) co_return;

    std::ifstream file(filepath);
    if (!file.is_open())
    {
        Debug::LogError("Failed to open file for reading: " + filepath, false);
        co_return;
    }

    json data;
    file >> data;
    file.close();

    // Clear existing data to prevent duplication
    shaderDescriptions.clear();
    materialDescs.clear();
    m_soundDescs.clear();
    m_prefabDescs.clear();
    texture2DFilePaths.clear();
    cubemapTextureFilePaths.clear();
    meshDescriptions.clear();

    // Deserialize shaders
    if (data.contains("shaders"))
    {
        for (auto& [key, value] : data["shaders"].items())
        {
            ShaderDesc shaderDesc;
            shaderDesc.vertexShaderFilePath = value["vertexShader"].get<std::string>();
            shaderDesc.fragmentShaderFilePath = value["fragmentShader"].get<std::string>();
            shaderDescriptions[key] = shaderDesc;
        }
    }

    // Deserialize physics materials
    if (data.contains("PhysicMaterials"))
    {
        for (auto& [uniqueID, physicMaterialJson] : data["PhysicMaterials"].items()) {
            physicsMaterialDescriptions[uniqueID] = physicMaterialJson;
        }
    }

    // Deserialize prefabs
    if (data.contains("prefabs"))
    {
        for (auto& [uniqueID, prefabJson] : data["prefabs"].items()) {
            m_prefabDescs[uniqueID] = prefabJson;
        }
    }

    // Deserialize sounds
    if (data.contains("sounds"))
    {
        for (auto& [uniqueID, value] : data["sounds"].items()) {

            SoundDesc soundDesc;
            if (value.contains("filepath"))
            {
                soundDesc.filepath = value["filepath"].get<string>();
            }
            m_soundDescs[uniqueID] = soundDesc;
        }
    }
    // Deserialize materials
    if (data.contains("materials"))
    {
        for (auto& [uniqueID, materialJson] : data["materials"].items()) {

            MaterialDesc materialDesc;
            if (materialJson.contains("shader")) 
            {
                materialDesc.shaderId = materialJson["shader"];
            }
            materialDesc.serializedData = materialJson;
            materialDescs[uniqueID] = materialDesc;
        }
    }

    // Deserialize texture2Ds
    if (data.contains("texture2Ds"))
    {
        for (auto& [key, path] : data["texture2Ds"].items())
        {
            texture2DFilePaths[key] = path;
        }
    }

    // Deserialize cubemaps
    if (data.contains("cubemaps"))
    {
        for (auto& [key, value] : data["cubemaps"].items())
        {
            cubemapTextureFilePaths[key] = value.get<std::vector<std::string>>();
        }
    }

    // Deserialize meshes with instance transforms
    if (data.contains("meshes"))
    {
        for (auto& [key, value] : data["meshes"].items())
        {
            MeshDesc desc;
            desc.filePath = value["filepath"].get<std::string>();

            if (value.contains("instances"))
            {
                for (const auto& instanceData : value["instances"])
                {
                    Transform t;
                    t.deserialize(instanceData);
                    desc.instanceTransforms.push_back(t);
                }
            }

            meshDescriptions[key] = desc;
        }
    }

    hasLoadedResources = true;
    co_return;
}

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
    return ResourcePtr();
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

    // --- Update saved data ---
    if (std::dynamic_pointer_cast<Shader>(resource)) {
        auto shaderIt = shaderDescriptions.find(oldId);
        if (shaderIt != shaderDescriptions.end()) {
            shaderDescriptions[newId] = shaderIt->second;
            shaderDescriptions.erase(shaderIt);
            Debug::Log("Shader renamed from '" + oldId + "' to '" + newId + "'");
        }
    }
    else if (std::dynamic_pointer_cast<Prefab>(resource)) {
        auto prefabIt = m_prefabDescs.find(oldId);
        if (prefabIt != m_prefabDescs.end()) {
            m_prefabDescs[newId] = prefabIt->second;
            m_prefabDescs.erase(prefabIt);
            Debug::Log("Prefab renamed from '" + oldId + "' to '" + newId + "'");
        }
    }
    else if (std::dynamic_pointer_cast<Sound>(resource)) {
        auto soundIt = m_soundDescs.find(oldId);
        if (soundIt != m_soundDescs.end()) {
            m_soundDescs[newId] = soundIt->second;
            m_soundDescs.erase(soundIt);
            Debug::Log("Sound renamed from '" + oldId + "' to '" + newId + "'");
        }
    }
    else if (std::dynamic_pointer_cast<Material>(resource)) {
        auto materialIt = materialDescs.find(oldId);
        if (materialIt != materialDescs.end()) {
            materialDescs[newId] = materialIt->second;
            materialDescs.erase(materialIt);
            Debug::Log("Material renamed from '" + oldId + "' to '" + newId + "'");
        }
    }
    else if (std::dynamic_pointer_cast<Mesh>(resource)) {
        auto meshIt = meshDescriptions.find(oldId);
        if (meshIt != meshDescriptions.end()) {
            meshDescriptions[newId] = meshIt->second;
            meshDescriptions.erase(meshIt);
            Debug::Log("Mesh renamed from '" + oldId + "' to '" + newId + "'");
        }
    }
    else if (std::dynamic_pointer_cast<Texture>(resource)) {
        // Replace in texture2DFilePaths (file path used as ID)
        auto itTex = texture2DFilePaths.find(oldId);
        if (itTex != texture2DFilePaths.end()) {
            texture2DFilePaths[newId] = itTex->second;
            texture2DFilePaths.erase(itTex);
            Debug::Log("Texture2D renamed from '" + oldId + "' to '" + newId + "'");
        }

        // Also check if it's a cubemap key
        auto cubeIt = cubemapTextureFilePaths.find(oldId);
        if (cubeIt != cubemapTextureFilePaths.end()) {
            cubemapTextureFilePaths[newId] = cubeIt->second;
            cubemapTextureFilePaths.erase(cubeIt);
            Debug::Log("TextureCubeMap renamed from '" + oldId + "' to '" + newId + "'");
        }

        // Update any cubemaps that reference the texture by value
        for (auto& [cubeID, facePaths] : cubemapTextureFilePaths) {
            for (auto& facePath : facePaths) {
                if (facePath == oldId)
                    facePath = newId;
            }
        }
    }

}

void ResourceManager::DeleteResource(const std::string& uniqueId)
{
    m_resourcesToDestroy.insert(uniqueId);
}

void ResourceManager::DeleteFromSavedData(const std::shared_ptr<Resource>& resource, const std::string& uniqueId)
{
    // For Shader
    if (std::dynamic_pointer_cast<Shader>(resource)) {
        auto shaderIt = shaderDescriptions.find(uniqueId);
        if (shaderIt != shaderDescriptions.end()) {
            shaderDescriptions.erase(shaderIt);
            Debug::Log("Deleted Shader: " + uniqueId);
        }
    }
    // For Prefab
    else if (std::dynamic_pointer_cast<Prefab>(resource)) {
        auto prefabIt = m_prefabDescs.find(uniqueId);
        if (prefabIt != m_prefabDescs.end()) {
            m_prefabDescs.erase(prefabIt);
            Debug::Log("Deleted Prefab: " + uniqueId);
        }
    }
    // For Sound
    else if (std::dynamic_pointer_cast<Sound>(resource)) {
        auto soundIt = m_soundDescs.find(uniqueId);
        if (soundIt != m_soundDescs.end()) {
            m_soundDescs.erase(soundIt);
            Debug::Log("Deleted Sound: " + uniqueId);
        }
    }
    // For Material
    else if (std::dynamic_pointer_cast<Material>(resource)) {
        auto materialIt = materialDescs.find(uniqueId);
        if (materialIt != materialDescs.end()) {
            materialDescs.erase(materialIt);
            Debug::Log("Deleted Material: " + uniqueId);
        }
    }
    // For Mesh
    else if (std::dynamic_pointer_cast<Mesh>(resource)) {
        auto meshIt = meshDescriptions.find(uniqueId);
        if (meshIt != meshDescriptions.end()) {
            meshDescriptions.erase(meshIt);
            Debug::Log("Deleted Mesh: " + uniqueId);
        }
    }
    // For Texture
    else if (std::dynamic_pointer_cast<Texture>(resource)) {
        // Remove from texture2D list
        auto itTex = texture2DFilePaths.find(uniqueId);
        if (itTex != texture2DFilePaths.end()) {
            texture2DFilePaths.erase(itTex);
            Debug::Log("Deleted Texture2D: " + uniqueId);
        }

        // Remove as a cubemap key
        auto cubeIt = cubemapTextureFilePaths.find(uniqueId);
        if (cubeIt != cubemapTextureFilePaths.end()) {
            cubemapTextureFilePaths.erase(cubeIt);
            Debug::Log("Deleted Cubemap Texture: " + uniqueId);
        }

        // Also remove from any cubemap references
        for (auto& [cubeID, facePaths] : cubemapTextureFilePaths) {
            for (auto& facePath : facePaths) {
                if (facePath == uniqueId)
                    facePath = ""; // or handle missing face logic
            }
        }
    }
}

void ResourceManager::CleanUp()
{
    m_mapResources.clear();
}

void ResourceManager::SetSelectedResource(ResourcePtr selectedResource)
{
    m_selectedResource = selectedResource;
}

void ResourceManager::ClearInstancesFromMeshes()
{
    // this was due to scene changes and not refreshing resources may cause issues
    // Iterate over all resources in the map
    for (auto& [key, resource] : m_mapResources)
    {
        // Check if the resource is of type Mesh
        MeshPtr meshPtr = std::dynamic_pointer_cast<Mesh>(resource);
        if (meshPtr)
        {
            meshPtr->clearInstances();
        }
    }
}

CoroutineTask ResourceManager::createResourcesFromDescs()
{
    if (hasCreatedResources) co_return;

    // Create Shaders
    for (const auto& [uniqueID, shaderDesc] : shaderDescriptions)
    {
        createShader(shaderDesc, uniqueID);
    }

    // Create Prefabs
    for (const auto& [uniqueID, prefabData] : m_prefabDescs)
    {
        createPrefab(uniqueID, prefabData);
    }

    // Create Sounds
    for (const auto& [uniqueID, soundDesc] : m_soundDescs)
    {
        createSound(soundDesc, uniqueID);
    }

    // Create Materials
    for (const auto& [uniqueID, materialDesc] : materialDescs)
    {
        createMaterial(materialDesc, uniqueID);
    }
    
    // Create Physics Materials
    for (const auto& [uniqueID, physicMaterialDesc] : physicsMaterialDescriptions)
    {
        createPhysicsMaterial(uniqueID, physicMaterialDesc);
    }

    // Create Meshes
    for (const auto& [uniqueID, meshDesc] : meshDescriptions)
    {
        createMesh(meshDesc, uniqueID);
    }

    // Create Texture2D resources
    for (const auto& [uniqueID, filepath] : texture2DFilePaths)
    {
        createTexture2DFromFile(filepath, uniqueID); // Change type as needed
    }

    // Create Cubemap Textures
    for (const auto& [uniqueID, filepaths] : cubemapTextureFilePaths)
    {
        createCubeMapTextureFromFile(filepaths, uniqueID);
    }

    hasCreatedResources = true;
    co_return;
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
            if (auto prefabPtr = std::dynamic_pointer_cast<Prefab>(resource)) 
            {
                prefabPtr->onDestroy();
            }
            if (m_selectedResource == resource) {
                m_selectedResource = nullptr;
            }
            DeleteFromSavedData(resource, id);
            m_mapResources.erase(it);
            Debug::Log("Deleted resource with ID: " + id);
        }
        else {
            Debug::Log("Attempted to delete non‑existent resource ID: " + id);
        }
    }
    m_resourcesToDestroy.clear();
}