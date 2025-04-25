/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Texture2D.cpp
Description : Texture2D class is a representation of a 2D texture to be used by the graphics engine class
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
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
    m_desc = desc;
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

    // Load the image data using stb_image.
    int width, height, nrChannels;
    unsigned char* data = stbi_load(m_path.c_str(), &width, &height, &nrChannels, 0);
    if (!data)
    {
        Debug::LogError("Texture failed to load at path: " + m_path, false);
        return;
    }

    m_textureData = data;
    m_textureSize = { width, height };
    m_numChannels = nrChannels;


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
    if (m_numChannels == 3) glChannels = GL_RGB; // 3 channels (RGB).
    else if (m_numChannels == 4) glChannels = GL_RGBA; // 4 channels (RGBA).

    // Specify the 2D texture image.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureSize.width, m_textureSize.height, 0, glChannels, GL_UNSIGNED_BYTE, m_textureData);

    // Generate mipmaps for the texture.
    glGenerateMipmap(GL_TEXTURE_2D);

    // Free the image data.
    stbi_image_free(data);
}

void Texture2D::OnInspectorGUI()
{
    ImGui::Text(("Texture2D Inspector - ID: " + m_uniqueID).c_str());
    ImGui::Separator();

    if (ImGui::BeginTable("TexturePath", 3,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg))
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Filepath:");
        ImGui::TableSetColumnIndex(1);
        if (m_path.empty()) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Not Assigned");
        }
        else {
            ImGui::TextUnformatted(m_path.c_str());
        }
        ImGui::TableSetColumnIndex(2);
        if (ImGui::Button("Browse##TexturePath")) {
            auto chosen = TossEngine::GetInstance().openFileDialog("*.png;*.jpg;*.tga");
            if (!chosen.empty()) {
                auto root = getProjectRoot();
                auto relPath = std::filesystem::relative(chosen, root);
                m_path = relPath.string();
                if (!m_path.empty()) GenerateTexture();
            }
        }

        ImGui::EndTable();
    }

    //static const char* filterOptions[] = { "Linear", "Nearest" };
    //static int minFilterIndex = 0;
    //static int magFilterIndex = 0;
    //
    //if (ImGui::Combo("Min Filter", &minFilterIndex, filterOptions, IM_ARRAYSIZE(filterOptions)))
    //{
    //    GLenum filter = (minFilterIndex == 0) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;
    //    glTextureParameteri(m_textureId, GL_TEXTURE_MIN_FILTER, filter);
    //}
    //
    //if (ImGui::Combo("Mag Filter", &magFilterIndex, filterOptions, IM_ARRAYSIZE(filterOptions)))
    //{
    //    GLenum filter = (magFilterIndex == 0) ? GL_LINEAR : GL_NEAREST;
    //    glTextureParameteri(m_textureId, GL_TEXTURE_MAG_FILTER, filter);
    //}
    //
    //static const char* wrapModes[] = { "Repeat", "Mirrored", "Clamp to Edge" };
    //static int wrapIndex = 0;
    //
    //if (ImGui::Combo("Wrap Mode", &wrapIndex, wrapModes, IM_ARRAYSIZE(wrapModes)))
    //{
    //    GLenum wrapMode = GL_REPEAT;
    //    if (wrapIndex == 1) wrapMode = GL_MIRRORED_REPEAT;
    //    else if (wrapIndex == 2) wrapMode = GL_CLAMP_TO_EDGE;
    //
    //    glTextureParameteri(m_textureId, GL_TEXTURE_WRAP_S, wrapMode);
    //    glTextureParameteri(m_textureId, GL_TEXTURE_WRAP_T, wrapMode);
    //}
}


bool Texture2D::Delete(bool deleteSelf)
{
    return false;
}

int Texture2D::getHeight()
{
    return m_desc.textureSize.height;
}

int Texture2D::getWidth()
{
    return m_desc.textureSize.width;
}

unsigned char* Texture2D::getData() const
{
    if (!m_desc.textureData)
    {
        Debug::LogError("Texture data is null", false);
    }
    return m_desc.textureData;
}

// Sets the texture wrapping mode to mirrored repeat.
void Texture2D::setMirrored()
{
    // Set the wrapping mode for the S (horizontal) and T (vertical) coordinates to mirrored repeat.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
}

// Sets the texture wrapping mode to clamp to edge.
void Texture2D::setClampToEdge()
{
    // Set the wrapping mode for the S (horizontal) and T (vertical) coordinates to clamp to edge.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Texture2D::resize(Rect newTextureSize)
{
    m_desc.textureSize = newTextureSize;

    // Determine number of color channels
    GLenum glChannels = (m_desc.numChannels == 4) ? GL_RGBA : GL_RGB;

    // Delete old texture to prevent memory leaks
    if (m_textureId != 0)
    {
        glDeleteTextures(1, &m_textureId);
    }

    // Generate and bind new texture
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Upload new texture data (or NULL if resizing without data)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_desc.textureSize.width, m_desc.textureSize.height, 0, glChannels, GL_UNSIGNED_BYTE, m_desc.textureData);

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Destructor for the Texture2D class.
Texture2D::~Texture2D()
{
    glDeleteTextures(1, &m_textureId);
}