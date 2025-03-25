/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Resource.h
Description : Resource class represents a generic resource such as a file, image or texture
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include <map>
#include <string>
#include "Utils.h"

// Forward declaration of ResourceManager class
class ResourceManager;

/**
 * @class Resource
 * @brief Represents a generic resource such as a file, image, or texture.
 */
class TOSSENGINE_API Resource
{
public:
    /**
     * @brief Constructor for the Resource class.
     * @param path The file path to the resource.
     * @param manager Pointer to the resource manager.
     */
    Resource(const string& path, const string& uniqueId, ResourceManager* manager);

    /**
     * @brief Destructor for the Resource class.
     */
    virtual ~Resource();

    /**
     * @brief Gets the file path of the resource.
     * @return The file path of the resource.
     */
    std::string getPath();
    const std::string& getUniqueID();

protected:
    std::string m_path; // The file path of the resource
    std::string m_uniqueID;  // Unique identifier for the resource
};