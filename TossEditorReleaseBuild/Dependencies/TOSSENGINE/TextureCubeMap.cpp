#include "TextureCubeMap.h"
#include <glew.h>

TextureCubeMap::TextureCubeMap(const TextureCubeMapDesc& desc, const string& filePath, ResourceManager* manager) : Texture(filePath, manager)
{
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureId);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Determine the number of color channels in the texture data.
    auto glChannels = GL_RGB;
    if (desc.numChannels == 3) glChannels = GL_RGB; // 3 channels (RGB).
    else if (desc.numChannels == 4) glChannels = GL_RGBA; // 4 channels (RGBA).

    // Loop through each face of the cubemap and set the texture data.
    for (uint i = 0; i < 6; ++i)
    {
        if (desc.textureData[i])
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, desc.textureSize.width, desc.textureSize.height, 0, glChannels, GL_UNSIGNED_BYTE, desc.textureData[i]);
        }
        else
        {
            std::cerr << "Cubemap texture data for face " << i << " is null." << std::endl;
        }
    }

    // Generate mipmaps for the cubemap texture.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Store the texture description.
    m_desc = desc;
}

TextureCubeMap::~TextureCubeMap()
{
    glDeleteTextures(1, &m_textureId); // Destroys the cubemap texture
}

void TextureCubeMap::OnInspectorGUI()
{
    ImGui::Text(("TextureCube Inspector - ID: " + m_uniqueID).c_str());
    ImGui::Separator();
}

bool TextureCubeMap::Delete(bool deleteSelf)
{
    return false;
}
