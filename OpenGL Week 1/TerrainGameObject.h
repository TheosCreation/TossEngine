#pragma once

#include "GraphicsGameObject.h"
#include "HeightMap.h"

class TerrainGameObject : public GraphicsGameObject
{
public:
    /**
     * @brief Called when the quad GameObject is created.
     */
    virtual void onCreate() override;

    /**
     * @brief Sets the uniform data for the shader.
     * @param data The uniform data to set.
     */
    virtual void setUniformData(UniformData data) override;

    /**
     * @brief Sets the shader for the quad GameObject.
     * @param shader A shared pointer to the shader.
     */
    void setShader(const ShaderPtr& shader);

    /**
     * @brief Generates the terrain mesh from a height map.
     * @param _heightMap A shared pointer to the height map.
     */
    void generateTerrainMesh(HeightMapPtr _heightMap);

    /**
     * @brief Called every frame to update the graphics.
     * @param data The uniform data to set.
     */
    void onGraphicsUpdate(UniformData data) override;

    /**
     * @brief Called during the geometry pass.
     * @param data The uniform data for the pass.
     */
    void onGeometryPass(UniformData data) override;

    /**
     * @brief Called during the shadow pass.
     * @param index The index of the shadow map or light source.
     */
    void onShadowPass(uint index) override;

protected:
    /**
     * @brief Smooths the height map data.
     * @param heightData A reference to the vector of height data.
     * @param width The width of the height map.
     * @param depth The depth of the height map.
     */
    void smoothHeightMap(std::vector<float>& heightData, uint width, uint depth);

private:
    VertexArrayObjectPtr m_mesh; // A shared pointer to the vertex array object representing the terrain
};