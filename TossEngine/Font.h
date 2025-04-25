#pragma once
#include "Resource.h"
#include "stb_truetype.h"

class Font : public Resource
{
public:
    Font(const FontDesc& desc, const string& filePath, const string& uniqueId, ResourceManager* manager);
    Font(const std::string& uid, ResourceManager* mgr);

    void onCreateLate() override;

    void OnInspectorGUI() override;
    uint getId() const;
    int getAtlasWidth() const;
    int getAtlasHeight() const;
    const std::vector<stbtt_bakedchar>& getCharData() const noexcept { return m_charData; }
private:
    int m_atlasHeight = 512;
    int m_atlasWidth = 512;
    float m_pixelHeight = 32.0f;

    uint m_textureId;
    vector<stbtt_bakedchar> m_charData;
    vector<unsigned char> m_ttfData;

    SERIALIZABLE_MEMBERS(m_atlasHeight, m_atlasWidth, m_pixelHeight)
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
