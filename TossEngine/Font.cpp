#include "Font.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "TossEngine.h"

Font::Font(const FontDesc& desc, const string& filePath, const string& uniqueId, ResourceManager* manager) : Resource(filePath, uniqueId, manager)
{
    m_charData.resize(95);
    unsigned char* bitmap = new unsigned char[desc.atlasWidth * desc.atlasHeight];

    int ret = stbtt_BakeFontBitmap(
        desc.ttfData.data(), 0,
        desc.pixelHeight,
        bitmap,
        desc.atlasWidth, desc.atlasHeight,
        32, 95,
        m_charData.data()
    );
    if (ret < 0) {
        delete[] bitmap;
        throw std::runtime_error("stbtt_BakeFontBitmap failed");
    }

    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
        desc.atlasWidth, desc.atlasHeight,
        0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete[] bitmap;
    m_atlasWidth = desc.atlasWidth;
    m_atlasHeight = desc.atlasHeight;
    m_pixelHeight = desc.pixelHeight;
}

Font::Font(const std::string& uid, ResourceManager* mgr) : Resource(uid, mgr)
{
}

void Font::onCreateLate()
{
    if (!m_path.empty()) GenerateFont();
}

void Font::GenerateFont()
{
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

    m_charData.resize(95);
    unsigned char* bitmap = new unsigned char[m_atlasWidth * m_atlasHeight];

    int ret = stbtt_BakeFontBitmap(
        m_ttfData.data(), 0,
        m_pixelHeight,
        bitmap,
        m_atlasWidth, m_atlasHeight,
        32, 95,
        m_charData.data()
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

    delete[] bitmap;

    isLoaded = true;
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