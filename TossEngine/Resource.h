/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Resource.h
Description : Represents a generic resource such as a file, image, mesh, or texture in the TossEngine.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include <map>
#include <string>
#include "Utils.h"
#include "ISelectable.h"
#include "Serializable.h"

// Forward declaration
class ResourceManager;

/**
 * @class Resource
 * @brief Represents a generic asset or file-based resource (e.g., textures, meshes, fonts).
 */
class TOSSENGINE_API Resource : public ISelectable, public Serializable
{
public:
    /**
     * @brief Constructs a Resource with a given file path and unique ID.
     * @param path The file path associated with the resource.
     * @param uniqueId A globally unique identifier for the resource.
     * @param manager A pointer to the ResourceManager instance.
     */
    Resource(const std::string& path, const std::string& uniqueId, ResourceManager* manager);

    /**
     * @brief Constructs a Resource when only the unique ID is known (e.g., during loading).
     * @param uid Unique identifier for the resource.
     * @param mgr A pointer to the ResourceManager instance.
     */
    Resource(const std::string& uid, ResourceManager* mgr);

    /**
     * @brief Virtual destructor.
     */
    virtual ~Resource() = default;

    /**
     * @brief Called when the resource is first created.
     */
    virtual void onCreate() {}

    /**
     * @brief Called after all resources are created (late initialization).
     */
    virtual void onCreateLate() {}

    /**
     * @brief Called when the resource is about to be destroyed.
     */
    virtual void onDestroy() {}

    /**
     * @brief Gets the file path of the resource.
     * @return The file path string.
     */
    std::string getPath();

    /**
     * @brief Gets the unique identifier for this resource.
     * @return The unique ID string.
     */
    const std::string& getUniqueID();

    /**
     * @brief Sets the unique identifier for this resource.
     * @param id The new unique ID.
     */
    void setUniqueID(const std::string& id);

    /**
     * @brief Indicates whether the resource has been loaded successfully.
     */
    bool isLoaded = false;

protected:
    ResourceManager* m_resourceManager = nullptr; //!< Pointer to the resource manager owning this resource.
    std::string m_path;                           //!< File path associated with the resource.
    std::string m_uniqueID;                       //!< Unique ID for identifying the resource.
};
