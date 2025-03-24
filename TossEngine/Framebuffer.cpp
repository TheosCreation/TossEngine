/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Framebuffer.cpp
Description : A class representing a frame buffer for rendering.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "Framebuffer.h"
#include "GraphicsEngine.h"
#include "Texture2D.h"
#include "Shader.h"
#include <glew.h>
#include <glfw3.h>

Framebuffer::Framebuffer(Vector2 _windowSize)
{
    m_size = _windowSize;


    // Generate and bind the framebuffer
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    //// Generate and bind the texture
    //glGenTextures(1, &RenderTexture);
    //glBindTexture(GL_TEXTURE_2D, RenderTexture);
    //
    //// Define the texture parameters
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)_windowSize.x, (int)_windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    Texture2DDesc textureDesc;
    textureDesc.textureSize = { (int)_windowSize.x, (int)_windowSize.y };
    textureDesc.numChannels = 4;
    RenderTexture = ResourceManager::GetInstance().createTexture2D(textureDesc);

    // Attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTexture->getId(), 0);

    // Generate and bind the renderbuffer for depth and stencil
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)_windowSize.x, (int)_windowSize.y);

    // Attach the renderbuffer to the framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

Framebuffer::Framebuffer(const Framebuffer& other)
{
    m_size = other.m_size;

    // Generate and bind a new framebuffer
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // Create a new texture for the framebuffer
    Texture2DDesc textureDesc;
    textureDesc.textureSize = { (int)m_size.x, (int)m_size.y };
    textureDesc.numChannels = 4;
    RenderTexture = ResourceManager::GetInstance().createTexture2D(textureDesc);

    // Attach the new texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTexture->getId(), 0);

    // Generate and bind a new renderbuffer
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)m_size.x, (int)m_size.y);

    // Attach the renderbuffer to the framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    // Check if the new framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Copied Framebuffer is not complete!" << std::endl;
    }

    // Unbind framebuffer and renderbuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}


Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &FBO);
    glDeleteRenderbuffers(1, &RBO);
}

void Framebuffer::onResize(Vector2 size)
{
    Resizable::onResize(size);
    
    // Bind the framebuffer to update its attachments
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // Resize the existing texture
    RenderTexture->resize(Rect((int)size.x, (int)size.y));

    // Attach the new texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTexture->getId(), 0);

    // Delete the existing renderbuffer
    glDeleteRenderbuffers(1, &RBO);

    // Generate and bind a new renderbuffer for depth and stencil with the updated size
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)size.x, (int)size.y);

    // Attach the new renderbuffer to the framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer resize failed: Framebuffer is not complete!" << std::endl;
    }

    // Unbind texture, framebuffer, and renderbuffer
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

// Bind the framebuffer
void Framebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glViewport(0, 0, (int)m_size.x, (int)m_size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Unbind the framebuffer
void Framebuffer::UnBind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::WriteDepth()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);  // Source framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);    // Default framebuffer as destination
    glBlitFramebuffer(0, 0, (int)m_size.x, (int)m_size.y,    // Source dimensions
        0, 0, (int)m_size.x, (int)m_size.y,    // Destination dimensions
        GL_DEPTH_BUFFER_BIT, GL_NEAREST);  // Mask and filter
}

void Framebuffer::PopulateShader(ShaderPtr shader)
{
    shader->setTexture2D(RenderTexture, 0, "Texture0");
}

uint Framebuffer::getId()
{
    return FBO;
}
