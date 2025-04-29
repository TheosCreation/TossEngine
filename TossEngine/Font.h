/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : Font.h
Description : Represents a font resource, including texture atlas generation and text mesh building.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once
#include "Resource.h"

/**
 * @class Font
 * @brief A font resource managing glyphs, a font atlas, and mesh generation for text rendering.
 */
class TOSSENGINE_API Font : public Resource
{
public:
    Font(const std::string& uid, ResourceManager* mgr);

    /**
     * @brief Final initialization after loading or creation.
     */
    void onCreateLate() override;

    /**
     * @brief Generates the font atlas texture from TTF data.
     */
    void GenerateFont();

    /**
     * @brief Builds a text mesh from a given string of characters.
     * @param text The text to build the mesh for.
     * @return TextMeshData containing vertices and indices for rendering.
     */
    TextMeshData buildTextMesh(const std::string& text) const;

    /**
     * @brief Renders an inspector GUI to modify font properties.
     */
    void OnInspectorGUI() override;

    /**
     * @brief Returns the OpenGL texture ID of the font atlas.
     */
    uint getId() const;

    /**
     * @brief Gets the width of the font atlas texture.
     */
    int getAtlasWidth() const;

    /**
     * @brief Gets the height of the font atlas texture.
     */
    int getAtlasHeight() const;

    /**
     * @brief Gets the pixel height used for this font.
     */
    float getPixelHeight() const;

private:
    int m_atlasHeight = 512;            //!< Height of the generated font atlas (pixels)
    int m_atlasWidth = 512;             //!< Width of the generated font atlas (pixels)
    float m_pixelHeight = 32.0f;         //!< Default pixel height used when rasterizing glyphs

    uint m_textureId = 0;                //!< OpenGL texture ID of the font atlas
    std::vector<Glyph> m_charData = std::vector<Glyph>(10);    //!< Glyph data (likely resized properly at runtime)
    std::vector<unsigned char> m_ttfData = std::vector<unsigned char>(10); //!< Raw TTF font binary data

    SERIALIZABLE_MEMBERS(m_atlasHeight, m_atlasWidth, m_pixelHeight, m_path)
};

REGISTER_RESOURCE(Font)

// --- JSON Serialization ---

inline void to_json(json& j, const FontPtr& font) {
    if (font)
    {
        j = json{ { "id", font->getUniqueID() } };
    }
    else {
        j = nullptr;
    }
}

inline void from_json(const json& j, FontPtr& font) {
    if (j.contains("id") && !j["id"].is_null())
        font = ResourceManager::GetInstance().get<Font>(j["id"].get<std::string>());
}
