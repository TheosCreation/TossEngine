/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : GeometryBuffer.h
Description : A class representing a geometry buffer for rendering.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Utils.h"

class Shader;

/**
 * @class GeometryBuffer
 * @brief A class representing a geometry buffer used for rendering.
 */
class GeometryBuffer
{
public:
	/**
	 * @brief Static method to get the singleton instance of the GeometryBuffer class.
	 * @return The singleton instance of the GeometryBuffer.
	 */
	static GeometryBuffer& GetInstance()
	{
		static GeometryBuffer instance;
		return instance;
	}

	/**
	 * @brief Public method to initialize the GeometryBuffer with the specified window size.
	 * @param _windowSize The size of the window for which the geometry buffer is being initialized.
	 */
	void Init(Vector2 _windowSize);

	// Delete copy constructor and assignment operator to prevent copying.
	GeometryBuffer(const GeometryBuffer&) = delete;
	GeometryBuffer& operator=(const GeometryBuffer&) = delete;

	/**
	* @brief Binds the geometry buffer for rendering.
	*/
	void Bind();
	
	/**
     * @brief Unbinds the geometry buffer.
     */
	void UnBind();

	/**
	 * @brief Writes depth information to the geometry buffer.
	 */
	void WriteDepth();

	/**
	* @brief Populates the specified shader with uniform data related to the geometry buffer.
	* @param _shader A shared pointer to the shader that will be populated with geometry buffer data.
	*/
	void PopulateShader(ShaderPtr _shader);

	/**
	 * @brief Resizes the geometry buffer based on the new window size.
	 * @param _windowSize The new size of the window.
	 */
	void Resize(Vector2 _windowSize);

private:
	/**
	 * @brief Private contructor for the GeometryBuffer class.
	 */
	GeometryBuffer() = default;

	/**
	 * @brief Private destructor for the GeometryBuffer class.
	 */
	~GeometryBuffer() = default;

	Vector2 m_size = Vector2(0.0f); // Stores the size of the geometry buffer.
	uint FBO = 0; // Frame Buffer Object identifier.

	// Texture identifiers for various data stored in the geometry buffer.
	uint Texture_Position = 0; // Texture for position data.
	uint Texture_Normal = 1; // Texture for normal data.
	uint Texture_AlbedoShininess = 2; // Texture for albedo and shininess data.
	uint Texture_Depth = 3; // Texture for depth data.
	uint Texture_Reflectivity = 4; // Texture for reflectivity data.
};

