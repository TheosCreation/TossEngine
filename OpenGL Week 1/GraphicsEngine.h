#pragma once
#include "Utils.h"
#include "Math.h"
#include "ResourceManager.h"

/**
 * @class GraphicsEngine
 * @brief Contains all the render functionality of OpenGL.
 */
class GraphicsEngine
{
public:
    /**
     * @brief Provides access to the singleton instance of GraphicsEngine.
     * @return A reference to the GraphicsEngine instance.
     */
    static GraphicsEngine& GetInstance()
    {
        static GraphicsEngine instance;
        return instance;
    }

    // Delete the copy constructor and assignment operator to prevent copying
    GraphicsEngine(const GraphicsEngine& other) = delete;
    GraphicsEngine& operator=(const GraphicsEngine& other) = delete;

    /**
     * @brief Creates a Vertex Array Object (VAO) without an Index Buffer.
     * @param vbDesc Description of the Vertex Buffer.
     * @return A shared pointer to the created Vertex Array Object.
     */
    VertexArrayObjectPtr createVertexArrayObject(const VertexBufferDesc& vbDesc);

    /**
     * @brief Creates a Vertex Array Object (VAO) with an Index Buffer.
     * @param vbDesc Description of the Vertex Buffer.
     * @param ibDesc Description of the Index Buffer.
     * @return A shared pointer to the created Vertex Array Object.
     */
    VertexArrayObjectPtr createVertexArrayObject(const VertexBufferDesc& vbDesc, const IndexBufferDesc& ibDesc);

    /**
     * @brief Creates a Shader program.
     * @param desc Description of the Shader.
     * @return A shared pointer to the created Shader.
     */
    ShaderPtr createShader(const ShaderDesc& desc);

    ShaderPtr createComputeShader(const string& computeShaderFilename);

    /**
     * @brief Clears the screen with the specified color.
     * @param color The color to clear the screen with.
     */
    void clear(const glm::vec4& color, bool clearDepth = true, bool clearStencil = true);

    /**
     * @brief Sets the face culling mode.
     * @param type The type of face culling to use.
     */
    void setFaceCulling(const CullType& type);

    /**
     * @brief Sets the depth function mode.
     * @param type The type of depth type to use.
     */
    void setDepthFunc(const DepthType& type);

    void setScissorSize(const Rect size);

    void setScissor(bool enabled);

    void setBlendFunc(const BlendType& srcType, const BlendType& dstType);

    /**
     * @brief Sets the winding order for front-facing polygons.
     * @param type The winding order to use.
     */
    void setWindingOrder(const WindingOrder& type);

    void setStencil(const StencilOperationType& type);


    /**
     * @brief Sets the viewport size.
     * @param size The size of the viewport.
     */
    void setViewport(const Vector2& size);

    /**
     * @brief Sets multisampling to enabled or disabled.
     * @param enabled If multisampling should be enabled or disabled.
     */
    void setMultiSampling(bool enabled);

    /**
     * @brief Sets the active Vertex Array Object (VAO).
     * @param vao A shared pointer to the Vertex Array Object to set.
     */
    void setVertexArrayObject(const VertexArrayObjectPtr& vao);
    void setVertexArrayObject(const uint vaoId);

    /**
     * @brief Sets the active Shader program.
     * @param program A shared pointer to the Shader program to set.
     */
    void setShader(const ShaderPtr& program);

    void setTexture2D(const uint textureId, uint slot, std::string bindingName);

    RenderingPath getRenderingPath();
    void setRenderingPath(RenderingPath newRenderingPath);


    /**
     * @brief Sets the active 2D Texture.
     * @param texture A shared pointer to the Texture2D to set.
     * @param slot The texture slot to bind the texture to.
     */
    void setTexture2D(const TexturePtr& texture, uint slot, std::string bindingName);

    /**
     * @brief Sets the active Cube Map Texture.
     * @param texture A shared pointer to the Texture2D to set.
     * @param slot The texture slot to bind the texture to.
     */
    void setTextureCubeMap(const TexturePtr& texture, uint slot, std::string bindingName);

    /**
     * @brief Draws triangles.
     * @param triangleType The type of triangles to draw.
     * @param vertexCount The number of vertices to draw.
     * @param offset The offset to start drawing from.
     */
    void drawTriangles(const TriangleType& triangleType, uint vertexCount, uint offset);

    /**
     * @brief Draws indexed triangles.
     * @param triangleType The type of triangles to draw.
     * @param indicesCount The number of indices to draw.
     */
    void drawIndexedTriangles(const TriangleType& triangleType, uint indicesCount);

    /**
     * @brief Draws indexed triangles with instancing.
     * @param triangleType The type of triangles to draw.
     * @param indicesCount The number of indices to draw.
     * @param instanceCount The number of instances to draw.
     */
    void drawIndexedTrianglesInstanced(const TriangleType& triangleType, uint indicesCount, int instanceCount);

private:
    ShaderPtr currentShader = nullptr;
    RenderingPath renderingPath = RenderingPath::Deferred;

    /**
     * @brief Private constructor to prevent external instantiation.
     */
    GraphicsEngine() = default;

    /**
     * @brief Private destructor to prevent external deletion.
     */
    ~GraphicsEngine() = default;
};