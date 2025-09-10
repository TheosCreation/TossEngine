/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : GraphicsEngine.cpp
Description : GraphicsEngine class contains all the render functionality of opengl
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "GraphicsEngine.h"

#include <ImGuizmo.h>
#include <imgui.h>
#include "VertexArrayObject.h"
#include "Shader.h"
#include "Texture2D.h"
#include "ShadowMap.h"
#include "TextureCubeMap.h"
#include "ProjectSettings.h"
#include "TossPlayerSettings.h"

#if defined(_WIN32)
#include <glew.h>
#include <glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#elif defined(__PROSPERO__)
// TODO:Implement
#endif

void GraphicsEngine::Init(ProjectSettingsPtr& projectSettings)
{
    if (isInitilized) return;

    m_renderingPath = projectSettings->renderingPath;
    isInitilized = true;
}

void GraphicsEngine::Init(TossPlayerSettingsPtr& playerSettings)
{
    if (isInitilized) return;

    m_renderingPath = playerSettings->renderingPath;
    isInitilized = true;
}

VertexArrayObjectPtr GraphicsEngine::createVertexArrayObject(const VertexBufferDesc& vbDesc)
{
    return std::make_shared<VertexArrayObject>(vbDesc);
}

VertexArrayObjectPtr GraphicsEngine::createVertexArrayObject(const VertexBufferDesc& vbDesc, const IndexBufferDesc& ibDesc)
{
    return std::make_shared<VertexArrayObject>(vbDesc, ibDesc);
}

void GraphicsEngine::updateVertexArrayObject(const VertexArrayObjectPtr& vao, const void* vertexData, uint dataSize)
{
#if defined(_WIN32)
    // Bind the VAO (assumes your VAO encapsulates its VBO(s))
    glBindVertexArray(vao->getId());

    // Bind the vertex buffer object associated with the VAO.
    // This assumes you have a way to retrieve the VBO handle from your VAO abstraction.
    glBindBuffer(GL_ARRAY_BUFFER, vao->getId());

    // Update the buffer data. We're assuming the buffer has been allocated with enough space.
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, vertexData);

    // Unbind the buffer and VAO for cleanliness.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::clear(const glm::vec4& color, bool clearDepth, bool clearStencil)
{
#if defined(_WIN32)
    glClearColor(color.x, color.y, color.z, color.w);

    // Start with clearing the color buffer
    GLbitfield clearFlags = GL_COLOR_BUFFER_BIT;

    // Add depth and stencil buffer bits based on the provided booleans
    if (clearDepth)
    {
        clearFlags |= GL_DEPTH_BUFFER_BIT;
    }
    if (clearStencil)
    {
        clearFlags |= GL_STENCIL_BUFFER_BIT;
    }

    // Clear the specified buffers
    glClear(clearFlags);
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::createImGuiFrame()
{
#ifdef __PROSPERO__
#else
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
#endif
    ImGui::NewFrame();
}

// Inside your GraphicsEngine class
ImGuiContext* GraphicsEngine::getImGuiContext()
{
    return ImGui::GetCurrentContext();
}


void GraphicsEngine::renderImGuiFrame()
{
    ImGui::Render();
#ifdef __PROSPERO__
#else
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}

void GraphicsEngine::setFaceCulling(const CullType& type)
{
    m_cullType = type;
#if defined(_WIN32)
    uint cullType = GL_BACK;
    if (type == CullType::None)
    {
        glDisable(GL_CULL_FACE);
    }
    else {
        if (type == CullType::FrontFace) cullType = GL_FRONT;
        else if (type == CullType::BackFace) cullType = GL_BACK;
        else if (type == CullType::Both) cullType = GL_FRONT_AND_BACK;

        glEnable(GL_CULL_FACE);
        glCullFace(cullType);
    }
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::setDepthFunc(const DepthType& type)
{
    m_depthType = type;

#if defined(_WIN32)
    uint depth = GL_LESS;
    switch (m_depthType)
    {
    case DepthType::Never:        depth = GL_NEVER; break;
    case DepthType::Less:         depth = GL_LESS; break;
    case DepthType::Equal:        depth = GL_EQUAL; break;
    case DepthType::LessEqual:    depth = GL_LEQUAL; break;
    case DepthType::Greater:      depth = GL_GREATER; break;
    case DepthType::NotEqual:     depth = GL_NOTEQUAL; break;
    case DepthType::GreaterEqual: depth = GL_GEQUAL; break;
    case DepthType::Always:       depth = GL_ALWAYS; break;
    }

    if (m_depthType == DepthType::Never)
    {
        m_depthTestingEnabled = false;
        glDisable(GL_DEPTH_TEST);
    }
    else
    {
        m_depthTestingEnabled = true;
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(depth);
    }
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::setDepthMask(bool writeEnabled)
{
    m_depthMaskEnabled = writeEnabled;

#if defined(_WIN32)
    glDepthMask(m_depthMaskEnabled ? GL_TRUE : GL_FALSE);
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::setDepthTest(bool testingEnabled)
{
    m_depthTestingEnabled = testingEnabled;

#if defined(_WIN32)
    if (m_depthTestingEnabled)
    {
        glEnable(GL_DEPTH_TEST);
        uint depth = GL_LESS; //TODO: fix this make it not need to copy everywhere
        switch (m_depthType)
        {
        case DepthType::Never:        depth = GL_NEVER; break;
        case DepthType::Less:         depth = GL_LESS; break;
        case DepthType::Equal:        depth = GL_EQUAL; break;
        case DepthType::LessEqual:    depth = GL_LEQUAL; break;
        case DepthType::Greater:      depth = GL_GREATER; break;
        case DepthType::NotEqual:     depth = GL_NOTEQUAL; break;
        case DepthType::GreaterEqual: depth = GL_GEQUAL; break;
        case DepthType::Always:       depth = GL_ALWAYS; break;
        }
        glDepthFunc(depth);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::setScissorSize(const Rect size)
{
#if defined(_WIN32)
    glScissor(size.left, size.top, size.width, size.height);
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::setScissor(bool enabled)
{
#if defined(_WIN32)
    if (enabled)
    {
        glEnable(GL_SCISSOR_TEST);
    }
    else
    {
        glDisable(GL_SCISSOR_TEST);
    }
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::setBlendFunc(const BlendType& srcType, const BlendType& dstType)
{
#if defined(_WIN32)
    GLenum blendType1 = GL_SRC_ALPHA; // Default source blend factor
    GLenum blendType2 = GL_ONE_MINUS_SRC_ALPHA; // Default destination blend factor

    // Determine the source blend factor
    switch (srcType)
    {
    case BlendType::Zero:                    blendType1 = GL_ZERO; break;
    case BlendType::One:                     blendType1 = GL_ONE; break;
    case BlendType::SrcColor:                blendType1 = GL_SRC_COLOR; break;
    case BlendType::OneMinusSrcColor:        blendType1 = GL_ONE_MINUS_SRC_COLOR; break;
    case BlendType::DstColor:                blendType1 = GL_DST_COLOR; break;
    case BlendType::OneMinusDstColor:        blendType1 = GL_ONE_MINUS_DST_COLOR; break;
    case BlendType::SrcAlpha:                blendType1 = GL_SRC_ALPHA; break;
    case BlendType::OneMinusSrcAlpha:        blendType1 = GL_ONE_MINUS_SRC_ALPHA; break;
    case BlendType::DstAlpha:                blendType1 = GL_DST_ALPHA; break;
    case BlendType::OneMinusDstAlpha:        blendType1 = GL_ONE_MINUS_DST_ALPHA; break;
    case BlendType::ConstantColor:           blendType1 = GL_CONSTANT_COLOR; break;
    case BlendType::OneMinusConstantColor:   blendType1 = GL_ONE_MINUS_CONSTANT_COLOR; break;
    case BlendType::ConstantAlpha:           blendType1 = GL_CONSTANT_ALPHA; break;
    case BlendType::OneMinusConstantAlpha:   blendType1 = GL_ONE_MINUS_CONSTANT_ALPHA; break;
    }

    // Determine the destination blend factor
    switch (dstType)
    {
    case BlendType::Zero:                    blendType2 = GL_ZERO; break;
    case BlendType::One:                     blendType2 = GL_ONE; break;
    case BlendType::SrcColor:                blendType2 = GL_SRC_COLOR; break;
    case BlendType::OneMinusSrcColor:        blendType2 = GL_ONE_MINUS_SRC_COLOR; break;
    case BlendType::DstColor:                blendType2 = GL_DST_COLOR; break;
    case BlendType::OneMinusDstColor:        blendType2 = GL_ONE_MINUS_DST_COLOR; break;
    case BlendType::SrcAlpha:                blendType2 = GL_SRC_ALPHA; break;
    case BlendType::OneMinusSrcAlpha:        blendType2 = GL_ONE_MINUS_SRC_ALPHA; break;
    case BlendType::DstAlpha:                blendType2 = GL_DST_ALPHA; break;
    case BlendType::OneMinusDstAlpha:        blendType2 = GL_ONE_MINUS_DST_ALPHA; break;
    case BlendType::ConstantColor:           blendType2 = GL_CONSTANT_COLOR; break;
    case BlendType::OneMinusConstantColor:   blendType2 = GL_ONE_MINUS_CONSTANT_COLOR; break;
    case BlendType::ConstantAlpha:           blendType2 = GL_CONSTANT_ALPHA; break;
    case BlendType::OneMinusConstantAlpha:   blendType2 = GL_ONE_MINUS_CONSTANT_ALPHA; break;
    }

    if (srcType == BlendType::Zero && dstType == BlendType::Zero)
    {
        glDisable(GL_BLEND);
    }
    else
    {
        glEnable(GL_BLEND);
        glBlendFunc(blendType1, blendType2);
    }
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::setWindingOrder(const WindingOrder& type)
{
#if defined(_WIN32)
    auto orderType = GL_CW;

    if (type == WindingOrder::ClockWise) orderType = GL_CW;
    else if (type == WindingOrder::CounterClockWise) orderType = GL_CCW;

    glFrontFace(orderType);
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::setStencil(const StencilOperationType& type)
{
#if defined(_WIN32)
    switch (type)
    {
    case StencilOperationType::Set:
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        break;

    case StencilOperationType::ResetNotEqual:
        glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
        glStencilMask(0x00);
        break;

    case StencilOperationType::ResetAlways:
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glStencilMask(0xFF);
        break;
    }
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::setViewport(const Vector2& size)
{
#if defined(_WIN32)
    glViewport(0, 0, (int)size.x, (int)size.y);
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::setMultiSampling(bool enabled)
{
#if defined(_WIN32)
    if (enabled)
    {
        glEnable(GL_MULTISAMPLE);
    }
    else
    {
        glDisable(GL_MULTISAMPLE);
    }
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::setVertexArrayObject(const VertexArrayObjectPtr& vao)
{
#if defined(_WIN32)
    glBindVertexArray(vao->getId());
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::setVertexArrayObject(const uint vaoId)
{
#if defined(_WIN32)
    glBindVertexArray(vaoId);
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::setShader(const ShaderPtr& _shader)
{
    m_currentActiveShader = _shader;
#if defined(_WIN32)
    glUseProgram(m_currentActiveShader->getId());
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::setTexture2D(const uint textureId, uint slot, std::string bindingName)
{
#if defined(_WIN32)
    m_currentActiveShader->setTexture2D(textureId, slot, bindingName);
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

RenderingPath GraphicsEngine::getRenderingPath()
{
    return m_renderingPath;
}

void GraphicsEngine::setRenderingPath(RenderingPath newRenderingPath)
{
    m_renderingPath = newRenderingPath;
}

void GraphicsEngine::setTexture2D(const Texture2DPtr& texture, uint slot, std::string bindingName) const
{
    m_currentActiveShader->setTexture2D(texture, slot, bindingName);
}

void GraphicsEngine::setTextureCubeMap(const TextureCubeMapPtr& texture, uint slot, std::string bindingName) const
{
    m_currentActiveShader->setTextureCubeMap(texture, slot, bindingName);
}

void GraphicsEngine::drawTriangles(const TriangleType& triangleType, uint vertexCount, uint offset)
{
#if defined(_WIN32)
    auto glTriType = GL_TRIANGLES;

    switch (triangleType)
    {
    case TriangleType::TriangleList: { glTriType = GL_TRIANGLES; break; }
    case TriangleType::TriangleStrip: { glTriType = GL_TRIANGLE_STRIP; break; }
    case TriangleType::Points: { glTriType = GL_POINTS; break; }
    }
    glDrawArrays(glTriType, offset, vertexCount);
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::drawIndexedTriangles(const TriangleType& triangleType, uint indicesCount)
{
#if defined(_WIN32)
    auto glTriType = GL_TRIANGLES;

    switch (triangleType)
    {
    case TriangleType::TriangleList: { glTriType = GL_TRIANGLES; break; }
    case TriangleType::TriangleStrip: { glTriType = GL_TRIANGLE_STRIP; break; }
    }
    glDrawElements(glTriType, indicesCount, GL_UNSIGNED_INT, nullptr);
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::drawIndexedTrianglesInstanced(const TriangleType& triangleType, uint indicesCount, int instanceCount)
{
#if defined(_WIN32)
    auto glTriType = GL_TRIANGLES;

    switch (triangleType)
    {
    case TriangleType::TriangleList: { glTriType = GL_TRIANGLES; break; }
    case TriangleType::TriangleStrip: { glTriType = GL_TRIANGLE_STRIP; break; }
    }
    glDrawElementsInstanced(glTriType, indicesCount, GL_UNSIGNED_INT, nullptr, instanceCount);
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::drawLines(const LineType& lineType, uint vertexCount, uint offset)
{
#if defined(_WIN32)
    GLenum glLineType = GL_LINES; // Default to GL_LINES
    switch (lineType)
    {
    case LineType::Lines:
        glLineType = GL_LINES;
        break;
    case LineType::LineStrip:
        glLineType = GL_LINE_STRIP;
        break;
    default:
        break;
    }
    glDrawArrays(glLineType, offset, vertexCount);
#elif defined(__PROSPERO__)
    // TODO:Implement
#endif
}

void GraphicsEngine::CleanUp()
{
    m_currentActiveShader.reset();
}