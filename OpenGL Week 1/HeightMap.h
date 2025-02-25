/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : 
Description : 
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Utils.h"
#include "Resource.h"

/**
 * @class HeightMap
 * @brief A representation of a HeightMap texture to be used by the graphics engine class.
 */
class HeightMap : public Resource
{
public:
    /**
     * @brief Constructor for the HeightMap class.
     * @param desc Description of the HeightMap texture.
     * @param path File path of the HeightMap texture.
     * @param manager Resource Manager of the HeightMap texture.
     */

    HeightMap(const HeightMapDesc& desc, const HeightMapInfo& buildInfo, const string& filePath, ResourceManager* manager);

    uint getWidth();
    uint getDepth();
    float getCellSpacing();

    std::vector<float> getData() const;

    /**
     * @brief Destructor for the HeightMap class.
     */
    ~HeightMap();

private:
    HeightMapDesc m_desc = {}; // Description of the HeightMap texture.
    HeightMapInfo m_buildInfo; // HeightMap build info.
};