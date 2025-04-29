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
***/

#pragma once
#include "Resource.h"

/**
 * @class Texture
 * @brief Abstract base class for texture resources, managing GPU texture IDs.
 */
class TOSSENGINE_API Texture : public Resource
{
public:
    /**
     * @brief Default constructor for an uninitialized texture.
     */
    Texture();

    /**
     * @brief Constructor with file path and unique ID.
     * @param filePath Path to the texture file.
     * @param uniqueId Unique identifier for the resource.
     * @param manager Resource manager managing this texture.
     */
    Texture(const std::string& filePath, const std::string& uniqueId, ResourceManager* manager);

    /**
     * @brief Constructor with unique ID (used for deserialization).
     * @param uniqueId Unique identifier.
     * @param manager Resource manager managing this texture.
     */
    Texture(const std::string& uniqueId, ResourceManager* manager);

    /**
     * @brief Destructor, cleans up OpenGL texture resources.
     */
    virtual ~Texture();

    /**
     * @brief Draws the texture's properties in the Inspector UI.
     */
    void OnInspectorGUI() override;

    /**
     * @brief Deletes the texture resource, optionally deleting itself from the manager.
     * @param deleteSelf Whether to also unregister from ResourceManager.
     * @return True if deleted successfully.
     */
    bool Delete(bool deleteSelf = true) override;

    /**
     * @brief Gets the OpenGL texture ID.
     * @return The OpenGL texture ID.
     */
    uint getId() const noexcept { return m_textureId; }

    /**
     * @brief Sets the OpenGL texture ID manually (should be used with caution).
     * @param id The new texture ID.
     */
    void setId(uint id) noexcept { m_textureId = id; }

protected:
    uint m_textureId = 0; //!< OpenGL ID of the texture.
};