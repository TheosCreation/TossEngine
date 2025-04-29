/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : GraphicsEngine.h
Description : Singleton wrapper for OpenGL functionality, including state management, VAO, shader, texture binding,
              and integration with ImGui. Used by the engine to abstract rendering details.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Utils.h"
#include "Math.h"
#include "ResourceManager.h"
#include "VertexArrayObject.h"

struct ImGuiContext;

/**
 * @class GraphicsEngine
 * @brief Central rendering engine for OpenGL state control, shader binding, draw calls, and ImGui rendering.
 */
class TOSSENGINE_API GraphicsEngine
{
public:
    /**
     * @brief Retrieves the singleton instance of GraphicsEngine.
     * @return Reference to the singleton.
     */
    static GraphicsEngine& GetInstance()
    {
        static GraphicsEngine instance;
        return instance;
    }

    // Disable copy/assignment to enforce singleton
    GraphicsEngine(const GraphicsEngine&) = delete;
    GraphicsEngine& operator=(const GraphicsEngine&) = delete;

    void Init(ProjectSettingsPtr& projectSettings);
    void Init(TossPlayerSettingsPtr& playerSettings);

    // --- Vertex Array Object Management ---

    VertexArrayObjectPtr createVertexArrayObject(const VertexBufferDesc& vbDesc);
    VertexArrayObjectPtr createVertexArrayObject(const VertexBufferDesc& vbDesc, const IndexBufferDesc& ibDesc);
    void updateVertexArrayObject(const VertexArrayObjectPtr& vao, const void* vertexData, uint dataSize);

    // --- State + Viewport ---

    void clear(const glm::vec4& color, bool clearDepth = true, bool clearStencil = true);
    void setViewport(const Vector2& size);
    void setScissorSize(const Rect size);
    void setScissor(bool enabled);
    void setMultiSampling(bool enabled);

    void setFaceCulling(const CullType& type);
    void setDepthFunc(const DepthType& type);
    void setWindingOrder(const WindingOrder& type);
    void setBlendFunc(const BlendType& srcType, const BlendType& dstType);
    void setStencil(const StencilOperationType& type);

    // --- ImGui Integration ---

    void createImGuiFrame();
    ImGuiContext* getImGuiContext();
    void renderImGuiFrame();

    // --- Draw Calls ---

    void drawTriangles(const TriangleType& triangleType, uint vertexCount, uint offset);
    void drawIndexedTriangles(const TriangleType& triangleType, uint indicesCount);
    void drawIndexedTrianglesInstanced(const TriangleType& triangleType, uint indicesCount, int instanceCount);
    void drawLines(const LineType& lineType, uint vertexCount, uint offset);

    // --- Shader + Texture Binding ---

    void setShader(const ShaderPtr& program);
    void setVertexArrayObject(const VertexArrayObjectPtr& vao);
    void setVertexArrayObject(uint vaoId);

    void setTexture2D(const Texture2DPtr& texture, uint slot, std::string bindingName) const;
    void setTexture2D(uint textureId, uint slot, std::string bindingName);
    void setTextureCubeMap(const TextureCubeMapPtr& texture, uint slot, std::string bindingName) const;

    // --- Rendering Mode ---

    RenderingPath getRenderingPath();
    void setRenderingPath(RenderingPath newRenderingPath);

    // --- Cleanup ---

    void CleanUp();

private:
    GraphicsEngine() = default;
    ~GraphicsEngine() = default;

    bool isInitilized = false;
    ShaderPtr currentShader = nullptr;
    RenderingPath m_renderingPath;
};
