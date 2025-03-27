#include "ShadowMap.h"

ShadowMap::ShadowMap(Vector2 _resolution) : Texture("", "", nullptr)
{
    m_resolution = _resolution;
    // Create the framebuffer object
    glGenFramebuffers(1, &FBO);

    // Create the depth texture
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, (int)_resolution.x, (int)_resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Attach the depth texture to the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_textureId, 0);

    // Set the framebuffer to have no color attachment (only depth)
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Debug::LogError("Framebuffer failed to initialize correctly", false);
    }

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

ShadowMap::~ShadowMap()
{
    glDeleteFramebuffers(1, &FBO);
    glDeleteTextures(1, &m_textureId);
}

void ShadowMap::Bind()
{
    // Bind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // Set the viewport to the shadowmaps size
    glViewport(0, 0, (int)m_resolution.x, (int)m_resolution.y);

    // Clears the depth
    glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::UnBind()
{
    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
