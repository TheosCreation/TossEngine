#pragma once
#include "Resource.h"
#include "stb_truetype.h"

class Font : public Resource
{
public:
    Font(const FontDesc& desc, const string& filePath, const string& uniqueId, ResourceManager* manager);
    Font(const std::string& uid, ResourceManager* mgr, const json& data);

    uint getId() const;
    int getAtlasWidth() const;
    int getAtlasHeight() const;
    const std::vector<stbtt_bakedchar>& getCharData() const noexcept { return m_charData; }
private:
    int m_atlasWidth;
    int m_atlasHeight;
    float m_pixelHeight;
    uint m_textureId;
    vector<stbtt_bakedchar> m_charData;
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
