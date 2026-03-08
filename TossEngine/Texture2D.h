/***
DeviousDevs
Auckland
New Zealand
(c) 2026 DeviousDevs
File Name : Texture2D.h
Description : Texture2D class is a representation of a 2D texture to be used by the graphics engine class
Author : Theo Morris
Mail : theo.morris@outlook.co.nz
**/

#pragma once
#include "Resource.h"

/**
 * @class Texture2D
 * @brief A representation of a 2D texture to be used by the graphics engine class.
 */
class TOSSENGINE_API Texture2D : public Resource
{
public:
    /**
     * @brief Constructor for the Texture2D class.
     * @param desc Description of the 2D texture.
     * @param path File path of the 2D texture.
     * @param manager Resource Manager of the 2D texture.
     */
    Texture2D(const Texture2DDesc& desc, const string& filePath, const string& uniqueId, ResourceManager* manager);
    Texture2D(const std::string& uid, ResourceManager* mgr);

    void onCreateLate() override;
    void GenerateTexture();

    void OnInspectorGUI() override;
    bool Delete(bool deleteSelf = true) override;

    /**
     * @brief Gets the height of the texture.
     * @return Height of the texture in pixels.
     */
    int getHeight() const;

    /**
     * @brief Gets the width of the texture.
     * @return Width of the texture in pixels.
     */
    int getWidth() const;

    /**
     * @brief Retrieves the raw pixel data of the texture.
     * @return Pointer to the texture data.
     */
    unsigned char* getData() const;

    /**
     * @brief Sets the texture wrapping mode to mirrored.
     * This mode mirrors the texture when it reaches its boundaries.
     */
    void setMirrored();

    /**
     * @brief Sets the texture wrapping mode to clamp to edge.
     * This mode clamps the texture to the edge pixel, preventing it from repeating.
     */
    void setClampToEdge();

    void resize(Rect newTextureSize);

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

    /**
     * @brief Destructor for the Texture2D class.
     * Cleans up resources associated with the texture.
     */
    ~Texture2D();

private:
    void ApplySamplerSettings() const;

    // TODO : stupid place for these probably move them
    static GLint FilterModeToMinFilter(int filterMode);
    static GLint FilterModeToMagFilter(int filterMode);
    static GLint WrapModeToGL(int wrapMode);
private:
    //Texture2DDesc m_desc = {}; // Description of the 2D texture.
    int m_numChannels = 3;
    Rect m_textureSize = {};

    int m_filterMode = static_cast<int>(TextureFilterMode::Linear);
    int m_wrapMode = static_cast<int>(TextureWrapMode::Repeat);
    
    uint m_textureId = 0;
    unsigned char* m_textureData = nullptr;

    SERIALIZABLE_MEMBERS(m_numChannels, m_textureSize, m_filterMode, m_wrapMode, m_path)
};

REGISTER_RESOURCE(Texture2D, ".meta", ".png", ".jpg", ".jpeg", ".tga", ".bmp");

inline void to_json(json& j, Texture2DPtr const& texture) {
    if (texture)
    {
        j = json{ { "id", texture->getUniqueID() } };
    }
    else {
        j = nullptr;
    }
}

inline void from_json(json const& j, Texture2DPtr& texture) {
    if (j.contains("id")) texture = ResourceManager::GetInstance().get<Texture2D>(j["id"].get<string>());
}