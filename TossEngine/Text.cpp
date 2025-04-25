#include "Text.h"
#include "GameObject.h"
#include "GraphicsEngine.h"
#include "TossEngine.h"

void Text::onCreateLate()
{
    RebuildMesh();
}

void Text::OnInspectorGUI()
{
    Renderer::OnInspectorGUI();

    if (ResourceAssignableField(m_font, "Font"))
    {
        RebuildMesh();
    }

    if (ImGui::InputText("Text", &m_text))
    {
        RebuildMesh();
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
        shader->setMat4("projectionMatrix", data.uiProjectionMatrix);
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

void Text::RebuildMesh()
{
    if (!m_font) return;

    std::vector<Vertex>   verts;
    std::vector<uint> idxs;
    float cursorX = 0, cursorY = 0;

    auto& font = *m_font;
    int   atlasW = font.getAtlasWidth();
    int   atlasH = font.getAtlasHeight();
    auto& chardata = font.getCharData();

    for (char c : m_text) {
        if (c < 32 || c >= 128) continue;
        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(
            chardata.data(), atlasW, atlasH,
            c - 32, &cursorX, &cursorY,
            &q, 1
        );

        // 4 verts per glyph
        uint base = (uint)verts.size();
        verts.push_back({ {q.x0, q.y0, 0}, {q.s0, q.t1} });
        verts.push_back({ {q.x1, q.y0, 0}, {q.s1, q.t1} });
        verts.push_back({ {q.x1, q.y1, 0}, {q.s1, q.t0} });
        verts.push_back({ {q.x0, q.y1, 0}, {q.s0, q.t0} });

        // 2 triangles
        idxs.push_back(base + 0);
        idxs.push_back(base + 1);
        idxs.push_back(base + 2);
        idxs.push_back(base + 2);
        idxs.push_back(base + 3);
        idxs.push_back(base + 0);
    }

    // 2) set up your attrib layout
    static const VertexAttribute attribs[] = {
        {3}, // pos
        {2}  // uv
    };

    // 3) upload
    m_vao = GraphicsEngine::GetInstance().createVertexArrayObject(
        // VBO
        {
          verts.data(),
          sizeof(Vertex),
          (uint)verts.size(),
          (VertexAttribute*)attribs,
          2
        },
        // IBO
      {
        idxs.data(),
        (uint)idxs.size()
      }
    );
}
