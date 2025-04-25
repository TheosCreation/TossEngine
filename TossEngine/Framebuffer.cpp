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
    int width = int(_windowSize.x);
    int height = int(_windowSize.y);

    // 1) Create & bind FBO
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // 2) Create the color texture
    glGenTextures(1, &RenderTexture);
    glBindTexture(GL_TEXTURE_2D, RenderTexture);
    // Allocate storage for the texture (RGBA8)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    // Set filtering/wrap
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Attach to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        RenderTexture,
        0);


    // 3) Create & bind RBO for depth+stencil
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER,
        GL_DEPTH24_STENCIL8,
        width,
        height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER,
        RBO);

    // 4) Check completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Framebuffer is not complete!" << std::endl;

    // 5) Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &FBO);
    glDeleteRenderbuffers(1, &RBO);
}

void Framebuffer::onResize(Vector2 newSize)
{
    if (newSize.x <= 2 || newSize.y <= 2) return;
    Resizable::onResize(newSize);
    
    int width = int(newSize.x);
    int height = int(newSize.y);
    
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    
    glBindTexture(GL_TEXTURE_2D, RenderTexture);
    glTexImage2D(GL_TEXTURE_2D,
        0,                  // mipmap level
        GL_RGBA8,           // internal format
        width, height,      // new dimensions
        0,                  // border
        GL_RGBA,            // format of provided data
        GL_UNSIGNED_BYTE,   // type of provided data
        nullptr);           // no initial data
    
    glFramebufferTexture2D(GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        RenderTexture,
        0);
    
    glDeleteRenderbuffers(1, &RBO);
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER,
        GL_DEPTH24_STENCIL8,
        width,
        height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER,
        RBO);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Framebuffer resize failed: not complete\n";
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
