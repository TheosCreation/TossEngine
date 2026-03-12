#include "TextFile.h"
#include "../ResourceManager.h"
#include "../Debug.h"
#include <fstream>
#include <sstream>
#include <imgui.h>

TextFile::TextFile(const std::string& uid, ResourceManager* mgr)
    : Resource(uid, mgr)
{
}

void TextFile::onCreateLate()
{
    m_content.clear();

    std::ifstream file(getPath(), std::ios::in);
    if (!file.is_open())
    {
        Debug::LogWarning("Failed to open text file: " + getPath());
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    m_content = buffer.str();
}

void TextFile::OnInspectorGUI()
{
    ImGui::Text("Path: %s", getPath().c_str());
    ImGui::Separator();

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_ReadOnly;
    ImGui::InputTextMultiline(
        "##TextFileContent",
        m_content.data(),
        m_content.size() + 1,
        ImVec2(-1.0f, 400.0f),
        flags
    );
}