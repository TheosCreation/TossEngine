/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : TextureCubeMap.h
Description : TextureCubeMap class is a resource that represents a cubemap texture used by the graphics engine
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Resource.h"

/**
 * @class TextureCubeMap
 * @brief A resource that represents a cubemap texture used by the graphics engine.
 */
class TOSSENGINE_API TextureCubeMap : public Resource
{
public:
    /**
     * @brief Constructor for the TextureCubeMap class.
     * @param faces The file paths to the six faces of the cubemap.
     * @param manager Pointer to the resource manager.
     */
    TextureCubeMap(const TextureCubeMapDesc& desc, const string& uniqueId, ResourceManager* manager);
    TextureCubeMap(const std::string& uid, ResourceManager* mgr);

    void onCreateLate() override;

    /**
     * @brief Destructor for the TextureCubeMap class.
     */
    ~TextureCubeMap();

    void OnInspectorGUI() override;

    /**
     * @brief Gets the ID of the texture.
     * @return The ID of the texture.
     */
    uint getId() const { return m_textureId; }

    /**
     * @brief Sets the ID of the texture.
     * @param id The new ID for the texture.
     */
    void setId(uint id) { m_textureId = id; }

private:
    uint m_textureId = 0;
    TextureCubeMapDesc m_desc = {};     // Description of the 2D texture.
    int m_numChannels = 3;
    Rect m_textureSize = {};
    vector<std::string> m_filePaths = vector<std::string>(6);

    vector<void*> m_textureDatas;

    SERIALIZABLE_MEMBERS(m_numChannels, m_textureSize, m_filePaths)
};

inline void to_json(json& j, TextureCubeMapPtr const& cubemap) {
    if (cubemap)
    {
        j = json{ { "id", cubemap->getUniqueID() } };
    }
    else {
        j = nullptr;
    }
}

inline void from_json(json const& j, TextureCubeMapPtr& cubemap) {
    if (j.contains("id")) cubemap = ResourceManager::GetInstance().getTextureCubeMap(j["id"].get<string>());
}
