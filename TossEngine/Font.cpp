#include "Font.h"
#include "TossEngine.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

Font::Font(const std::string& uid, ResourceManager* mgr) : Resource(uid, mgr)
{
}

void Font::onCreateLate()
{
    if (!m_path.empty()) GenerateFont();
}

void Font::GenerateFont()
{
    vector<stbtt_bakedchar> baked;
    baked.resize(95);
    std::ifstream ifs(m_path, std::ios::binary | std::ios::ate);
    if (!ifs) {
        Debug::LogError("Font file not found: " + m_path, false);
        return;
    }

    auto fileSize = ifs.tellg();
    ifs.seekg(0);

    m_ttfData.resize((size_t)fileSize);
    ifs.read(reinterpret_cast<char*>(m_ttfData.data()), fileSize);
    ifs.close();

    unsigned char* bitmap = new unsigned char[m_atlasWidth * m_atlasHeight];

    int ret = stbtt_BakeFontBitmap(
        m_ttfData.data(), 0,
        m_pixelHeight,
        bitmap,
        m_atlasWidth, m_atlasHeight,
        32, 95,
        baked.data()
    );
    if (ret < 0) {
        delete[] bitmap;
        throw std::runtime_error("stbtt_BakeFontBitmap failed");
    }

    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
        m_atlasWidth, m_atlasHeight,
        0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    m_charData.clear();
    m_charData.reserve(baked.size());
    for (auto& b : baked) {
        Glyph g;
        g.x0 = static_cast<unsigned short>(b.x0);
        g.y0 = static_cast<unsigned short>(b.y0);
        g.x1 = static_cast<unsigned short>(b.x1);
        g.y1 = static_cast<unsigned short>(b.y1);
        g.xoff = b.xoff;
        g.yoff = b.yoff;
        g.xadvance = b.xadvance;
        m_charData.push_back(g);
    }

    delete[] bitmap;

    isLoaded = true;
}

TextMeshData Font::buildTextMesh(const std::string& text) const
{
    TextMeshData out;
    float cursorX = 0, cursorY = 0;
    int atlasW = getAtlasWidth();
    int atlasH = getAtlasHeight();

    // 1) Actually size the array so data() is valid
    std::vector<stbtt_bakedchar> baked;
    baked.resize(m_charData.size());

    // 2) Copy your Glyph back into true stbtt_bakedchar (unsigned short!)
    for (size_t i = 0; i < m_charData.size(); ++i) {
        auto const& g = m_charData[i];
        stbtt_bakedchar& b = baked[i];
        b.x0 = static_cast<unsigned short>(g.x0);
        b.y0 = static_cast<unsigned short>(g.y0);
        b.x1 = static_cast<unsigned short>(g.x1);
        b.y1 = static_cast<unsigned short>(g.y1);
        b.xoff = g.xoff;
        b.yoff = g.yoff;
        b.xadvance = g.xadvance;
    }

    // 3) Generate quads safely
    for (char c : text) {
        if (c < 32 || c >= 128) continue;

        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(
            baked.data(), atlasW, atlasH,
            c - 32, &cursorX, &cursorY,
            &q, 1
        );

        uint32_t base = uint32_t(out.verts.size());
        out.verts.push_back({ {q.x0, q.y0, 0}, {q.s0, q.t1} });
        out.verts.push_back({ {q.x1, q.y0, 0}, {q.s1, q.t1} });
        out.verts.push_back({ {q.x1, q.y1, 0}, {q.s1, q.t0} });
        out.verts.push_back({ {q.x0, q.y1, 0}, {q.s0, q.t0} });

        out.idxs.insert(out.idxs.end(), {
            base + 0, base + 1, base + 2,
            base + 2, base + 3, base + 0
            });
    }

    return out;
}

void Font::OnInspectorGUI()
{
    ImGui::Text(("Font Inspector - ID: " + m_uniqueID).c_str());
    ImGui::Separator();

    if (ImGui::BeginTable("FontFilepath", 3,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg))
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Filepath:");
        ImGui::TableSetColumnIndex(1);
        if (m_path.empty()) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Not Assigned");
        }
        else {
            ImGui::TextUnformatted(m_path.c_str());
        }
        ImGui::TableSetColumnIndex(2);
        if (ImGui::Button("Browse##FontFilepath")) {
            auto chosen = TossEngine::GetInstance().openFileDialog("*.ttf;*.otf");
            if (!chosen.empty()) {
                auto root = getProjectRoot();
                auto relPath = std::filesystem::relative(chosen, root);
                m_path = relPath.string();
                if (!m_path.empty()) GenerateFont();
            }
        }

        ImGui::EndTable();
    }

    if (FloatSliderField("Pixel Height", m_pixelHeight))
    {
        GenerateFont();
    }
    if (IntSliderField("Atlas Width", m_atlasWidth))
    {
        GenerateFont();
    }
    if (IntSliderField("Atlas Height", m_atlasHeight))
    {
        GenerateFont();
    }
}

uint Font::getId() const
{
    return m_textureId;
}

int Font::getAtlasWidth() const
{
    return m_atlasWidth;
}

int Font::getAtlasHeight() const
{
    return m_atlasHeight;
}

float Font::getPixelHeight() const
{
    return m_pixelHeight;
}
