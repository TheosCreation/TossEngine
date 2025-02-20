/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Framebuffer.h
Description : A class representing a frame buffer for rendering.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Utils.h"

/**
 * @class Framebuffer
 * @brief A class representing a framebuffer used for rendering operations.
 */
class Framebuffer
{
public: 
	/**
	 * @brief Constructor for the Framebuffer class.
	 * @param _windowSize The initial size of the framebuffer.
	 */
	Framebuffer(Vector2 _windowSize);

	/**
	 * @brief Destructor for the Framebuffer class.
	 */
	~Framebuffer();

	/**
	 * @brief Resizes the framebuffer to the new specified window size.
	 * @param _newWindowSize The new size for the framebuffer.
	 */
	void Resize(Vector2 _newWindowSize);

	/**
	 * @brief Binds the framebuffer for rendering.
	 */
	void Bind();

	/**
	 * @brief Unbinds the framebuffer, restoring the default framebuffer.
	 */
	void UnBind();

	uint RenderTexture; // Identifier for the texture used for rendering.

private:
	uint FBO; // Frame Buffer Object identifier.
	uint RBO; // Render Buffer Object identifier.
	Vector2 m_size = Vector2(0); // Current size of the framebuffer.
};

