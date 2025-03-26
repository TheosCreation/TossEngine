/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Texture.h
Description : Texture class handles the loading and management of texture resources.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Resource.h"

/**
 * @class Texture
 * @brief Handles the loading and management of texture resources.
 */
class TOSSENGINE_API Texture : public Resource
{
public:
    /**
     * @brief Constructor for the Texture class.
     * @param path The file path to the texture.
     * @param manager Pointer to the resource manager.
     */
    Texture();

    /**
     * @brief Constructor for the Texture class with a file path.
     * @param path The file path to the texture.
     * @param manager Pointer to the resource manager.
     */
    Texture(const string& filePath, const string& uniqueId, ResourceManager* manager);

    void OnInspectorGUI() override;
    bool Delete(bool deleteSelf = true) override;

    /**
     * @brief Destructor for the Texture class.
     * Cleans up resources associated with the texture.
     */
    ~Texture();

    /**
     * @brief Gets the ID of the texture.
     * @return The ID of the texture.
     */
    uint getId() const;

    /**
     * @brief Sets the ID of the texture.
     * @param id The new ID for the texture.
     */
    void setId(uint id);

protected:
    uint m_textureId = 0; // The ID of the texture.
};