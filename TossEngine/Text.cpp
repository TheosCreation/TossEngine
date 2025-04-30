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
    UiElement::OnInspectorGUI();

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
        Mat4 modelMatrix = m_owner->m_transform.GetMatrix();

        if (m_isUi)
        {
            Vector2 screenSize = data.uiScreenSize;
            Vector2 textSize = GetTextSize();
            Vector2 anchorOffset = GetAnchorOffset(screenSize, textSize, m_anchorPoint);
            Vector2 pivotOffset = GetPivotOffsetFromCenter(textSize);

            modelMatrix = Mat4::Translate(Vector3(anchorOffset.x, -anchorOffset.y, 0.0f)) * modelMatrix;
            modelMatrix = Mat4::Translate(Vector3(-pivotOffset.x, -pivotOffset.y, 0.0f)) * modelMatrix;

            shader->setMat4("VPMatrix", data.uiProjectionMatrix);
        }
        else
        {
            Vector2 textSize = GetTextSize();
            Vector2 pivotOffset = GetPivotOffsetFromCenter(textSize);
            modelMatrix = modelMatrix * Mat4::Translate(Vector3(-pivotOffset.x, -pivotOffset.y, 0.0f));
            shader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
        }

        shader->setMat4("modelMatrix", modelMatrix);
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
Vector2 Text::GetTextSize() const
{
    if (!m_font) return Vector2(0, 0);

    float totalWidth = 0.0f;
    float maxHeight = 0.0f;

    for (char c : m_text)
    {
        Glyph glyph = m_font->getGlyph(c);
        totalWidth += glyph.xadvance;

        float glyphHeight = static_cast<float>(glyph.y1 - glyph.y0);
        float glyphYOff = glyph.yoff;
        float totalGlyphHeight = glyphHeight + glyphYOff;

        if (totalGlyphHeight > maxHeight)
            maxHeight = totalGlyphHeight;
    }

    return Vector2(totalWidth, maxHeight);
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
Vector2 Text::GetPivotOffsetFromCenter(Vector2 textSize) const
{
    Vector2 offset;
    switch (m_pivotPoint)
    {
    case TopLeft:      offset = Vector2(0.0f, textSize.y); break;
    case TopCenter:    offset = Vector2(0.5f * textSize.x, textSize.y); break;
    case TopRight:     offset = Vector2(textSize.x, textSize.y); break;
    case MiddleLeft:   offset = Vector2(0.0f, 0.5f * textSize.y); break;
    case Center:       offset = Vector2(0.5f * textSize.x, 0.5f * textSize.y); break;
    case MiddleRight:  offset = Vector2(textSize.x, 0.5f * textSize.y); break;
    case BottomLeft:   offset = Vector2(0.0f, 0.0f); break;
    case BottomCenter: offset = Vector2(0.5f * textSize.x, 0.0f); break;
    case BottomRight:  offset = Vector2(textSize.x, 0.0f); break;
    }
    return offset;
}
