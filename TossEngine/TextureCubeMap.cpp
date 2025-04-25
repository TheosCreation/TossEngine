#include "TextureCubeMap.h"
#include <glew.h>
#include "TossEngine.h"

#include "stb_image.h"

TextureCubeMap::TextureCubeMap(const TextureCubeMapDesc& desc, const string& uniqueId, ResourceManager* manager) : Resource(uniqueId, uniqueId, manager)
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

TextureCubeMap::TextureCubeMap(const std::string& uid, ResourceManager* mgr) : Resource(uid, mgr)
{
}

void TextureCubeMap::onCreateLate()
{
    bool canGenerate = true;
    for (const auto& filepath : m_filePaths)
    {
        if (filepath.empty()) canGenerate = false;
    }
    if (canGenerate) GenerateTextureCubeMap();
}

void TextureCubeMap::GenerateTextureCubeMap()
{
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureId);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (uint i = 0; i < 6; ++i)
    {
        int w, h, n;
        unsigned char* data = stbi_load(m_filePaths[i].c_str(), &w, &h, &n, 0);
        if (!data)
        {
            Debug::LogError("Cubemap face failed to load: " + m_filePaths[i], false);
            return;
        }

        GLenum fmt = (n == 4 ? GL_RGBA : GL_RGB);
        // internal format == fmt is fine here
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,                // mip level
            fmt,              // internal format
            w, h,
            0,                // border
            fmt,              // source format
            GL_UNSIGNED_BYTE, // type
            data              // pixel data
        );

        // free *after* the upload
        stbi_image_free(data);

        // store size + channels on first face
        if (i == 0)
        {
            m_textureSize = { w, h };
            m_numChannels = n;
        }
    }

    // Generate mipmaps for the cubemap texture.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void TextureCubeMap::onDestroy()
{
    glDeleteTextures(1, &m_textureId); // Destroys the cubemap texture
}

void TextureCubeMap::OnInspectorGUI()
{
    ImGui::Text(("TextureCube Inspector - ID: " + m_uniqueID).c_str());
    ImGui::Separator();

    static const std::vector<std::string> labels =
    {
        "Right (+X)", "Left (–X)",
        "Top (+Y)",   "Bottom (–Y)",
        "Back (+Z)", "Front (–Z)"
    };
    if (FileSelectionTableField("CubeMap", labels, m_filePaths, "*.png;*.jpg;*.tga"))
    {
        GenerateTextureCubeMap();
    }
}
