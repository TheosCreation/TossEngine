/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : GeometryBuffer.cpp
Description : A class representing a geometry buffer for rendering.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "GeometryBuffer.h"
#include "Shader.h"

void GeometryBuffer::Init(Vector2 _windowSize)
{
	m_size = _windowSize;

	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// Create the Position texture
	glGenTextures(1, &Texture_Position);
	glBindTexture(GL_TEXTURE_2D, Texture_Position);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (int)_windowSize.x, (int)_windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Create the Normal texture
	glGenTextures(1, &Texture_Normal);
	glBindTexture(GL_TEXTURE_2D, Texture_Normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (int)_windowSize.x, (int)_windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Create the Albedo/Shininess texture
	glGenTextures(1, &Texture_AlbedoShininess);
	glBindTexture(GL_TEXTURE_2D, Texture_AlbedoShininess);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)_windowSize.x, (int)_windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Create the Depth texture
	glGenTextures(1, &Texture_Depth);
	glBindTexture(GL_TEXTURE_2D, Texture_Depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, (int)_windowSize.x, (int)_windowSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Attach the textures to the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture_Position, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, Texture_Normal, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, Texture_AlbedoShininess, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Texture_Depth, 0);

	// Define the color buffers as output targets for the fragment shader
	GLuint Attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, Attachments);

	// Check if the framebuffer is still complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Debug::LogError("Framebuffer is not complete after resizing!");
	}

	// Unbindings
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GeometryBuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glViewport(0, 0, (int)m_size.x, (int)m_size.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GeometryBuffer::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GeometryBuffer::WriteDepth()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);  // Source framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);    // Default framebuffer as destination
	glBlitFramebuffer(0, 0, (int)m_size.x, (int)m_size.y,    // Source dimensions
					  0, 0, (int)m_size.x, (int)m_size.y,    // Destination dimensions
					  GL_DEPTH_BUFFER_BIT, GL_NEAREST);  // Mask and filter
}

void GeometryBuffer::PopulateShader(ShaderPtr _shader)
{
	_shader->setTexture2D(Texture_Position, 0, "Texture_Position");
	_shader->setTexture2D(Texture_Normal, 1, "Texture_Normal");
	_shader->setTexture2D(Texture_AlbedoShininess, 2, "Texture_AlbedoShininess"); 
	//_shader->setTexture2D(Texture_Depth, 3, "Texture_Depth");
}

void GeometryBuffer::Resize(Vector2 _windowSize)
{
	m_size = _windowSize;

	// Update the Position texture
	glBindTexture(GL_TEXTURE_2D, Texture_Position);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (int)_windowSize.x, (int)_windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Update the Normal texture
	glBindTexture(GL_TEXTURE_2D, Texture_Normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (int)_windowSize.x, (int)_windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Update the Albedo/Shininess texture
	glBindTexture(GL_TEXTURE_2D, Texture_AlbedoShininess);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)_windowSize.x, (int)_windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Update the Depth texture
	glBindTexture(GL_TEXTURE_2D, Texture_Depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, (int)_windowSize.x, (int)_windowSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Bind the framebuffer to update its attachments
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture_Position, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, Texture_Normal, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, Texture_AlbedoShininess, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Texture_Depth, 0);

	// Check if the framebuffer is still complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Debug::LogError("Framebuffer is not complete after resizing!");
	}

	// Unbindings
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
