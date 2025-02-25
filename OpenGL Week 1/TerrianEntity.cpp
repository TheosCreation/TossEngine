#include "TerrainEntity.h"
#include "GraphicsEngine.h"
#include "VertexArrayObject.h"
#include "LightManager.h"

void TerrainEntity::generateTerrainMesh(HeightMapPtr _heightMap)
{
    if (!_heightMap) {
        // Handle the error if _heightMap is null
        Debug::LogError("Heightmap is null could not generate terrain");
        return;
    }

    // Retrieve height map data
    std::vector<float> data = _heightMap->getData();
    uint depth = _heightMap->getDepth();
    uint width = _heightMap->getWidth();
    float cellSpacing = _heightMap->getCellSpacing();

    // Apply smoothing to the height map multiple times to create a smoother terrain
    smoothHeightMap(data, width, depth);
    smoothHeightMap(data, width, depth);
    smoothHeightMap(data, width, depth);
    smoothHeightMap(data, width, depth);
    smoothHeightMap(data, width, depth);

    // Calculate the number of vertices and indices for the terrain mesh
    uint numVertices = width * depth;
    uint numIndices = (width - 1) * (depth - 1) * 6;

    // Allocate memory for the vertices and indices arrays
    Vertex* verticesList = new Vertex[numVertices];
    uint* indicesList = new uint[numIndices];

    // Precompute half the width and depth for easier positioning
    float halfWidth = (width - 1) * cellSpacing * 0.5f;
    float halfDepth = (depth - 1) * cellSpacing * 0.5f;
    float texU = 1.0f / (width - 1); // Texture coordinate scaling
    float texV = 1.0f / (depth - 1); // Texture coordinate scaling

    // Generate vertices for the terrain grid
    for (uint row = 0; row < depth; ++row) {
        float posZ = halfDepth - (row * cellSpacing); // Z position based on row
        for (uint col = 0; col < width; ++col) {
            uint i = row * width + col;
            float posX = -halfWidth + (col * cellSpacing); // X position based on column
            float posY = data[i]; // Y position from height data

            // Set vertex properties
            verticesList[i].position = Vector3(posX, posY, posZ);
            verticesList[i].texCoords = Vector2(col * texU, row * texV);
            verticesList[i].normal = Vector3(0.0f, 1.0f, 0.0f); // Default normal pointing up
        }
    }

    // Generate indices for the terrain grid (triangular mesh)
    uint k = 0;
    for (uint row = 0; row < (width - 1); ++row) {
        for (uint col = 0; col < (depth - 1); ++col) {
            // First triangle of the quad
            indicesList[k] = row * depth + col;
            indicesList[k + 1] = row * depth + col + 1;
            indicesList[k + 2] = (row + 1) * depth + col;

            // Second triangle of the quad
            indicesList[k + 3] = (row + 1) * depth + col;
            indicesList[k + 4] = row * depth + col + 1;
            indicesList[k + 5] = (row + 1) * depth + col + 1;

            k += 6; // Move to the next set of indices
        }
    }

    // Calculate normals using central difference for each vertex
    float invCellSpacing = 1.0f / (2.0f * cellSpacing);
    for (uint row = 0; row < width; ++row) {
        for (uint col = 0; col < depth; ++col) {
            // Sample surrounding height values for calculating the normal
            float rowNeg = data[(row == 0 ? row : row - 1) * depth + col];
            float rowPos = data[(row == width - 1 ? row : row + 1) * depth + col];
            float colNeg = data[row * depth + (col == 0 ? col : col - 1)];
            float colPos = data[row * depth + (col == depth - 1 ? col : col + 1)];

            // Calculate gradients in X and Y directions
            float x = (rowNeg - rowPos);
            if (row == 0 || row == width - 1) {
                x *= 2.0f; // Double the gradient at boundaries
            }

            float y = (colPos - colNeg);
            if (col == 0 || col == depth - 1) {
                y *= 2.0f; // Double the gradient at boundaries
            }

            // Compute tangents and normal using cross product
            Vector3 tangentZ(0.0f, x * invCellSpacing, 1.0f);
            Vector3 tangentX(1.0f, y * invCellSpacing, 0.0f);
            Vector3 normal = glm::normalize(glm::cross(tangentZ, tangentX));
            unsigned int i = row * depth + col;
            verticesList[i].normal = normal; // Set the normal for the current vertex
        }
    }

    // Define vertex attributes for position, texture coordinates, and normals
    static const VertexAttribute attribsList[] = {
        { 3 }, // numElements for position attribute
        { 2 }, // numElements for texture coordinates attribute
        { 3 }  // numElements for normal attribute
    };

    // Create the vertex array object using the vertex and index buffers
    m_mesh = GraphicsEngine::GetInstance().createVertexArrayObject(
        // Vertex buffer
        {
                (void*)verticesList,
                sizeof(Vertex), // Size in bytes of a single composed vertex
                numVertices,  // Number of vertices

                (VertexAttribute*)attribsList,
                3 // Number of elements in the attribute list
        },
        // Index buffer
        {
            (void*)indicesList,
            numIndices // Number of indices
        }
    );

    // Free allocated memory for vertex and index arrays
    delete[] verticesList;
    delete[] indicesList;
}

void TerrainEntity::onCreate()
{
    // Override function for handling terrain entity creation
}

void TerrainEntity::setUniformData(UniformData data)
{
    // Override function for setting uniform data for the shader
}

void TerrainEntity::setShader(const ShaderPtr& shader)
{
    m_shader = shader; // Set the shader used by the terrain entity
}

void TerrainEntity::onGraphicsUpdate(UniformData data)
{
    // Call base class update
    GraphicsEntity::onGraphicsUpdate(data);

    // Set uniform matrices for shader
    m_shader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
    m_shader->setMat4("modelMatrix", m_transform.GetMatrix());

    // Get graphics engine instance
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setFaceCulling(CullType::None);
    graphicsEngine.setWindingOrder(WindingOrder::ClockWise);
    LightManager::GetInstance().applyLighting(m_shader);

    // Bind textures if available
    if (m_texture != nullptr)
        graphicsEngine.setTexture2D(m_texture, 0, "Texture0");

    if (m_texture1 != nullptr)
        graphicsEngine.setTexture2D(m_texture1, 1, "Texture1");

    if (m_texture2 != nullptr)
        graphicsEngine.setTexture2D(m_texture2, 2, "Texture2");

    if (m_texture3 != nullptr)
        graphicsEngine.setTexture2D(m_texture3, 3, "Texture3");

    if (m_heightMap != nullptr)
        graphicsEngine.setTexture2D(m_heightMap, 4, "HeightMap");

    // Apply shadows and draw the mesh
    auto& lightManager = LightManager::GetInstance();
    lightManager.applyShadows(m_shader);
    graphicsEngine.setVertexArrayObject(m_mesh);
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, m_mesh->getNumIndices());

    // Unbind textures
    for (int i = 0; i <= 5; ++i)
        graphicsEngine.setTexture2D(nullptr, i, "");
}

void TerrainEntity::onGeometryPass(UniformData data)
{
    // Perform geometry pass setup
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setFaceCulling(CullType::None);
    graphicsEngine.setWindingOrder(WindingOrder::ClockWise);
    graphicsEngine.setDepthFunc(DepthType::Less);
    graphicsEngine.setShader(m_geometryShader);

    // Set shader matrices
    m_geometryShader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
    m_geometryShader->setMat4("modelMatrix", m_transform.GetMatrix());

    // Bind textures for geometry pass
    if (m_texture != nullptr)
        graphicsEngine.setTexture2D(m_texture, 0, "Texture0");

    if (m_texture1 != nullptr)
        graphicsEngine.setTexture2D(m_texture1, 1, "Texture1");

    if (m_texture2 != nullptr)
        graphicsEngine.setTexture2D(m_texture2, 2, "Texture2");

    if (m_texture3 != nullptr)
        graphicsEngine.setTexture2D(m_texture3, 3, "Texture3");

    if (m_heightMap != nullptr)
        graphicsEngine.setTexture2D(m_heightMap, 4, "HeightMap");

    // Draw the terrain mesh
    graphicsEngine.setVertexArrayObject(m_mesh);
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, m_mesh->getNumIndices());

    // Unbind textures after rendering
    for (int i = 0; i <= 5; ++i)
        graphicsEngine.setTexture2D(nullptr, i, "");
}

void TerrainEntity::onShadowPass(uint index)
{
    GraphicsEntity::onShadowPass(index);

    // Perform shadow pass setup
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setFaceCulling(CullType::None);
    graphicsEngine.setWindingOrder(WindingOrder::ClockWise);

    // Set shader matrices
    auto& lightManager = LightManager::GetInstance();
    m_shadowShader->setMat4("VPLight", lightManager.getLightSpaceMatrix(index));
    m_shadowShader->setMat4("modelMatrix", m_transform.GetMatrix());

    if (m_mesh == nullptr) return;

    // Draw the mesh to update the shadow map
    graphicsEngine.setVertexArrayObject(m_mesh);
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, m_mesh->getNumIndices());
}

void TerrainEntity::smoothHeightMap(std::vector<float>& heightData, uint width, uint depth)
{
    std::vector<float> smoothedData(heightData.size());

    // Apply a simple box blur filter
    for (uint row = 0; row < depth; ++row) {
        for (uint col = 0; col < width; ++col) {
            float sum = 0.0f;
            uint count = 0;

            // Iterate over a 3x3 grid around the current cell
            for (int r = -1; r <= 1; ++r) {
                for (int c = -1; c <= 1; ++c) {
                    uint newRow = std::clamp<int>(row + r, 0, depth - 1);
                    uint newCol = std::clamp<int>(col + c, 0, width - 1);
                    sum += heightData[newRow * width + newCol];
                    count++;
                }
            }

            smoothedData[row * width + col] = sum / count;
        }
    }

    // Copy the smoothed data back to the original heightData
    heightData = std::move(smoothedData);
}
