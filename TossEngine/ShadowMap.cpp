#include "ShadowMap.h"

ShadowMap::ShadowMap(Vector2 _resolution) : Texture("", "", nullptr)
{
    m_resolution = _resolution;
    // Create the framebuffer object
    glCreateFramebuffers(1, &FBO);

    // Create the depth texture
    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureId);
    glTextureStorage2D(m_textureId, 1, GL_DEPTH_COMPONENT24, (int)_resolution.x, (int)_resolution.y);

    // Set texture parameters directly (no bind required)
    glTextureParameteri(m_textureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_textureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_textureId, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Attach the depth texture to the framebuffer
    glNamedFramebufferTexture(FBO, GL_DEPTH_ATTACHMENT, m_textureId, 0);

    // Set framebuffer to have no color outputs
    glNamedFramebufferDrawBuffer(FBO, GL_NONE);
    glNamedFramebufferReadBuffer(FBO, GL_NONE);

    // Check if framebuffer is complete
    if (glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Debug::LogError("Framebuffer failed to initialize correctly", false);
    }
}

ShadowMap::~ShadowMap()
{
    glDeleteFramebuffers(1, &FBO);
    glDeleteTextures(1, &m_textureId);
}

void ShadowMap::Bind()
{
    glGetIntegerv(GL_VIEWPORT, m_prevViewport);

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

    // Reset the viewport size
    glViewport(m_prevViewport[0], m_prevViewport[1], m_prevViewport[2], m_prevViewport[3]);
}
