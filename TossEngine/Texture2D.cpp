/***
DeviousDevs
Auckland
New Zealand
(c) 2026 DeviousDevs
File Name : Texture2D.cpp
Description : Texture2D class is a representation of a 2D texture to be used by the graphics engine class
Author : Theo Morris
Mail : theo.morris@outlook.co.nz
**/

#include "Texture2D.h"
#include <glew.h>
#include "TossEngine.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Constructor that initializes a 2D texture with the given description.
Texture2D::Texture2D(const Texture2DDesc& desc, const string& filePath, const string& uniqueId, ResourceManager* manager) : Resource(filePath, uniqueId, manager)
{
    // Generate a texture ID and bind it as a 2D texture.
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    // Set texture wrapping parameters.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repeat texture horizontally.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repeat texture vertically.

    // Set texture filtering parameters.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Use linear filtering and generate mipmaps.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Use linear filtering for magnification.

    // Determine the number of color channels in the texture data.
    auto glChannels = GL_RGB;
    if (desc.numChannels == 3) glChannels = GL_RGB; // 3 channels (RGB).
    else if (desc.numChannels == 4) glChannels = GL_RGBA; // 4 channels (RGBA).

    // Specify the 2D texture image.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, desc.textureSize.width, desc.textureSize.height, 0, glChannels, GL_UNSIGNED_BYTE, desc.textureData);

    // Generate mipmaps for the texture.
    glGenerateMipmap(GL_TEXTURE_2D);

    // Store the texture description.
    //m_desc = desc;
}

Texture2D::Texture2D(const std::string& uid, ResourceManager* mgr) : Resource(uid, mgr)
{
}

void Texture2D::onCreateLate()
{
    if (!m_path.empty()) GenerateTexture();
}

void Texture2D::GenerateTexture()
{
    stbi_set_flip_vertically_on_load(false);

    int width = 0;
    int height = 0;
    int nrChannels = 0;
    unsigned char* data = stbi_load(m_path.c_str(), &width, &height, &nrChannels, 0);
    if (!data)
    {
        Debug::LogError("Texture failed to load at path: " + m_path, false);
        return;
    }

    m_textureData = data;
    m_textureSize = { width, height };
    m_numChannels = nrChannels;

    if (m_textureId != 0)
    {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
    }

    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    GLenum glChannels = GL_RGB;
    if (m_numChannels == 4)
    {
        glChannels = GL_RGBA;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureSize.width, m_textureSize.height, 0, glChannels, GL_UNSIGNED_BYTE, m_textureData);
    glGenerateMipmap(GL_TEXTURE_2D);

    ApplySamplerSettings();

    stbi_image_free(data);
    m_textureData = nullptr;
}

void Texture2D::OnInspectorGUI()
{
    ImGui::Text(("Texture2D Inspector - ID: " + m_uniqueID).c_str());
    ImGui::Separator();

    if (ImGui::BeginTable("TexturePath", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg))
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Filepath:");

        ImGui::TableSetColumnIndex(1);
        if (m_path.empty())
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Not Assigned");
        }
        else
        {
            ImGui::TextUnformatted(m_path.c_str());
        }

        ImGui::TableSetColumnIndex(2);
        if (ImGui::Button("Browse##TexturePath"))
        {
            std::string chosen = TossEngine::GetInstance().openFileDialog("*.png;*.jpg;*.tga");
            if (!chosen.empty())
            {
                std::filesystem::path root = getProjectRoot();
                std::filesystem::path relativePath = std::filesystem::relative(chosen, root);
                m_path = relativePath.string();

                if (!m_path.empty())
                {
                    GenerateTexture();
                }
            }
        }

        ImGui::EndTable();
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Info");
    ImGui::Text("Size: %d x %d", m_textureSize.width, m_textureSize.height);
    ImGui::Text("Channels: %d", m_numChannels);
    ImGui::Text("OpenGL ID: %u", m_textureId);

    ImGui::Separator();
    ImGui::TextUnformatted("Preview");

    if (m_textureId != 0)
    {
        float availableWidth = ImGui::GetContentRegionAvail().x;
        float previewWidth = availableWidth;
        float previewHeight = previewWidth;

        if (m_textureSize.width > 0 && m_textureSize.height > 0)
        {
            float aspectRatio = static_cast<float>(m_textureSize.width) / static_cast<float>(m_textureSize.height);
            previewHeight = previewWidth / aspectRatio;

            const float maxPreviewHeight = 256.0f;
            if (previewHeight > maxPreviewHeight)
            {
                previewHeight = maxPreviewHeight;
                previewWidth = previewHeight * aspectRatio;
            }
        }

        ImGui::Image(m_textureId, ImVec2(previewWidth, previewHeight));
    }
    else
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "No texture loaded");
    }
}


bool Texture2D::Delete(bool deleteSelf)
{
    return false;
}

int Texture2D::getHeight() const
{
    return m_textureSize.height;
}

int Texture2D::getWidth() const
{
    return m_textureSize.width;
}

unsigned char* Texture2D::getData() const
{
    if (!m_textureData)
    {
        Debug::LogError("Texture data is null", false);
    }
    return m_textureData;
}

void Texture2D::setMirrored()
{
    m_wrapMode = static_cast<int>(TextureWrapMode::MirroredRepeat);
    ApplySamplerSettings();
}

void Texture2D::setClampToEdge()
{
    m_wrapMode = static_cast<int>(TextureWrapMode::ClampToEdge);
    ApplySamplerSettings();
}

void Texture2D::resize(Rect newTextureSize)
{
    m_textureSize = newTextureSize;

    GLenum glChannels = (m_numChannels == 4) ? GL_RGBA : GL_RGB;

    if (m_textureId != 0)
    {
        glDeleteTextures(1, &m_textureId);
    }

    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    ApplySamplerSettings();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureSize.width, m_textureSize.height, 0, glChannels, GL_UNSIGNED_BYTE, m_textureData);

    glBindTexture(GL_TEXTURE_2D, 0);
}

// Destructor for the Texture2D class.
Texture2D::~Texture2D()
{
    glDeleteTextures(1, &m_textureId);
}

void Texture2D::ApplySamplerSettings() const
{
    if (m_textureId == 0)
    {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, m_textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FilterModeToMinFilter(m_filterMode));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FilterModeToMagFilter(m_filterMode));

    GLint wrapMode = WrapModeToGL(m_wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    glBindTexture(GL_TEXTURE_2D, 0);
}

GLint Texture2D::FilterModeToMinFilter(int filterMode)
{
    if (filterMode == static_cast<int>(TextureFilterMode::Nearest))
    {
        return GL_NEAREST_MIPMAP_NEAREST;
    }

    return GL_LINEAR_MIPMAP_LINEAR;
}

GLint Texture2D::FilterModeToMagFilter(int filterMode)
{
    if (filterMode == static_cast<int>(TextureFilterMode::Nearest))
    {
        return GL_NEAREST;
    }

    return GL_LINEAR;
}

GLint Texture2D::WrapModeToGL(int wrapMode)
{
    if (wrapMode == static_cast<int>(TextureWrapMode::MirroredRepeat))
    {
        return GL_MIRRORED_REPEAT;
    }

    if (wrapMode == static_cast<int>(TextureWrapMode::ClampToEdge))
    {
        return GL_CLAMP_TO_EDGE;
    }

    return GL_REPEAT;
}
