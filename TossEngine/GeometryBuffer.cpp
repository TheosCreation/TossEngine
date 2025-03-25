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
	if (isInitilized) return;

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

	// Create the Reflectivity texture
	glGenTextures(1, &Texture_Reflectivity);
	glBindTexture(GL_TEXTURE_2D, Texture_Reflectivity);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, (int)_windowSize.x, (int)_windowSize.y, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Attach the textures to the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture_Position, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, Texture_Normal, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, Texture_AlbedoShininess, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, Texture_Reflectivity, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Texture_Depth, 0);

	// Define the color buffers as output targets for the fragment shader
	GLuint Attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, Attachments);

	// Check if the framebuffer is still complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Debug::LogError("Framebuffer is not complete after resizing!");
	}

	// Unbindings
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	isInitilized = true;
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

void GeometryBuffer::WriteDepth(uint writeBuffer)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, writeBuffer); // write to default framebuffer if FBO is not passed in
	glBlitFramebuffer(
		0, 0, (int)m_size.x, (int)m_size.y, 0, 0, (int)m_size.x, (int)m_size.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST
	);
}

void GeometryBuffer::PopulateShader(ShaderPtr _shader)
{
	_shader->setTexture2D(Texture_Position, 0, "Texture_Position");
	_shader->setTexture2D(Texture_Normal, 1, "Texture_Normal");
	_shader->setTexture2D(Texture_AlbedoShininess, 2, "Texture_AlbedoShininess"); 
	_shader->setTexture2D(Texture_Depth, 3, "Texture_Depth");
	_shader->setTexture2D(Texture_Reflectivity, 4, "Texture_Reflectivity");
}

void GeometryBuffer::Resize(Vector2 _windowSize)
{
    if (_windowSize.x <= 2 || _windowSize.y <= 2) return;

	// Store new size
	m_size = _windowSize;

	// Delete old framebuffer and textures
	glDeleteFramebuffers(1, &FBO);
	glDeleteTextures(1, &Texture_Position);
	glDeleteTextures(1, &Texture_Normal);
	glDeleteTextures(1, &Texture_AlbedoShininess);
	glDeleteTextures(1, &Texture_Depth);
	glDeleteTextures(1, &Texture_Reflectivity);

	isInitilized = false;
	// Reinitialize the framebuffer and textures
	Init(_windowSize);
}
