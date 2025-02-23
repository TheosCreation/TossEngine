#include "HeightMap.h"
#include <glew.h>

HeightMap::HeightMap(const HeightMapDesc& desc, const HeightMapInfo& buildInfo, const char* path, ResourceManager* manager) : Resource(path, "", manager)
{
    // Store the heightmap description.
    m_desc = desc;
    m_buildInfo = buildInfo;
}

uint HeightMap::getWidth()
{
    return m_buildInfo.width;
}

uint HeightMap::getDepth()
{
    return m_buildInfo.depth;
}

float HeightMap::getCellSpacing()
{
    return m_buildInfo.cellSpacing;
}

std::vector<float> HeightMap::getData() const
{
    return m_desc.data;
}

HeightMap::~HeightMap()
{
}
