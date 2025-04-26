#pragma once
#include "Resource.h"

class TOSSENGINE_API Font : public Resource
{
public:
    Font(const std::string& uid, ResourceManager* mgr);

    void onCreateLate() override;

    void GenerateFont();
    TextMeshData buildTextMesh(const std::string& text) const;

    void OnInspectorGUI() override;
    uint getId() const;
    int getAtlasWidth() const;
    int getAtlasHeight() const;
    float getPixelHeight() const;
private:
    int m_atlasHeight = 512;
    int m_atlasWidth = 512;
    float m_pixelHeight = 32.0f;

    uint m_textureId = 0;
    std::vector<Glyph> m_charData = std::vector<Glyph>(10);
    vector<unsigned char> m_ttfData = std::vector<unsigned char>(10);

    SERIALIZABLE_MEMBERS(m_atlasHeight, m_atlasWidth, m_pixelHeight, m_path)
};
REGISTER_RESOURCE(Font)

inline void to_json(json& j, FontPtr const& font) {
    if (font)
    {
        j = json{ { "id", font->getUniqueID() } };
    }
    else {
        j = nullptr;
    }
}

inline void from_json(json const& j, FontPtr& font) {
    if (j.contains("id")) font = ResourceManager::GetInstance().get<Font>(j["id"].get<string>());
}
