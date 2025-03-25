/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Texture2D.h
Description : Texture2D class is a representation of a 2D texture to be used by the graphics engine class
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Utils.h"
#include "Texture.h"

/**
 * @class Texture2D
 * @brief A representation of a 2D texture to be used by the graphics engine class.
 */
class TOSSENGINE_API Texture2D : public Texture
{
public:
    /**
     * @brief Constructor for the Texture2D class.
     * @param desc Description of the 2D texture.
     * @param path File path of the 2D texture.
     * @param manager Resource Manager of the 2D texture.
     */
    Texture2D(const Texture2DDesc& desc, const string& filePath, ResourceManager* manager);

    void OnInspectorGUI() override;
    bool Delete(bool deleteSelf = true) override;

    /**
     * @brief Gets the height of the texture.
     * @return Height of the texture in pixels.
     */
    int getHeight();

    /**
     * @brief Gets the width of the texture.
     * @return Width of the texture in pixels.
     */
    int getWidth();

    /**
     * @brief Retrieves the raw pixel data of the texture.
     * @return Pointer to the texture data.
     */
    unsigned char* getData() const;

    /**
     * @brief Sets the texture wrapping mode to mirrored.
     * This mode mirrors the texture when it reaches its boundaries.
     */
    void setMirrored();

    /**
     * @brief Sets the texture wrapping mode to clamp to edge.
     * This mode clamps the texture to the edge pixel, preventing it from repeating.
     */
    void setClampToEdge();

    void resize(Rect newTextureSize);

    /**
     * @brief Destructor for the Texture2D class.
     * Cleans up resources associated with the texture.
     */
    ~Texture2D();

private:
    Texture2DDesc m_desc = {}; // Description of the 2D texture.
};