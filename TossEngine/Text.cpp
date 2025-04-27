#include "Text.h"
#include "GameObject.h"
#include "GraphicsEngine.h"
#include "TossEngine.h"

void Text::onCreateLate()
{
    if (m_font && m_font->isLoaded)
        RebuildMesh();
}

void Text::OnInspectorGUI()
{
    Renderer::OnInspectorGUI();

    if (ResourceAssignableField(m_font, "Font"))
    {
        RebuildMesh();
    }
    ImGui::Checkbox("Is Ui", &m_isUi);
    if (ImGui::InputText("Text", &m_text))
    {
        if (!m_text.empty())
        {
            RebuildMesh();
        }
    }

    if(IntSliderField("Font Size", m_fontSize))
    {
        if (!m_text.empty())
        {
            RebuildMesh();
        }
    }
    ImGui::ColorEdit3("Color", m_color.Data());
}

void Text::Render(UniformData data, RenderingPath renderPath)
{
    if (!m_material || !m_font || !m_vao) return;

    ShaderPtr shader = m_material->GetShader();
    // Set the shader for the lighting pass
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setShader(shader);

    //have to check if it has an owner because image is used from drawing the scene and game view windows
    if (m_owner)
    {
        shader->setMat4("modelMatrix", m_owner->m_transform.GetMatrix());
        if (m_isUi)
        {
            shader->setMat4("VPMatrix", data.uiProjectionMatrix);
        }
        else
        {
            shader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
        }
    }

    if (m_font)
    {
        shader->setTexture2D(m_font->getId(), 0, "text");
    }
    shader->setVec3("textColor", m_color);

    // Prepare the graphics engine for rendering
    graphicsEngine.setFaceCulling(CullType::None); // Disable face culling
    graphicsEngine.setWindingOrder(WindingOrder::ClockWise); // Set winding order
    graphicsEngine.setVertexArrayObject(m_vao); // Bind the vertex array object for the mesh
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, m_vao->getNumIndices()); // Draw the indexed triangles
}

void Text::SetText(const string text)
{
    m_text = text;
    RebuildMesh();
}

string Text::GetText() const
{
    return m_text;
}

bool Text::GetIsUi() const
{
    return m_isUi;
}

void Text::RebuildMesh()
{
    if (!m_font || m_text.empty()) return;

    // 1) get the ready-made mesh
    TextMeshData data = m_font->buildTextMesh(m_text);

    // 2) figure out scale factor
    float originalH = m_font->getPixelHeight();
    float scale = m_fontSize / originalH;

    // 3) apply scale to all positions
    for (auto& v : data.verts) {
        v.position.x *= scale;
        v.position.y *= scale;
    }

    // 4) set up attrib layout & upload
    static const VertexAttribute attribs[] = {
        { 3 }, // pos
        { 2 }  // uv
    };
    m_vao = GraphicsEngine::GetInstance().createVertexArrayObject(
        // VBO
        {
            data.verts.data(),
            sizeof(Vertex),
            (uint)data.verts.size(),
            const_cast<VertexAttribute*>(attribs),
            2
        },
        // IBO
        {
            data.idxs.data(),
            (uint)data.idxs.size()
        }
    );
}
