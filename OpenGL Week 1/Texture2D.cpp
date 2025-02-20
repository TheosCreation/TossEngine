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

// Constructor that initializes a 2D texture with the given description.
Texture2D::Texture2D(const Texture2DDesc& desc, const char* path, ResourceManager* manager) : Texture(path, manager)
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
        Debug::LogError("Texture data is null");
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

// Destructor for the Texture2D class.
Texture2D::~Texture2D()
{
}