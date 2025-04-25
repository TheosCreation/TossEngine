#include "Sound.h"
#include "AudioEngine.h"
#include "TossEngine.h"

Sound::Sound(const SoundDesc& desc, const std::string& uniqueID, ResourceManager* manager) : Resource(desc.filepath, uniqueID, manager)
{
	m_desc = desc;
}

Sound::Sound(const std::string& uid, ResourceManager* mgr) : Resource(uid, mgr)
{
}

void Sound::OnInspectorGUI()
{
    ImGui::Text(("Sound Inspector - ID: " + m_uniqueID).c_str());
    ImGui::Separator();

    if (ImGui::BeginTable("SoundFilepath", 3,
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
        if (ImGui::Button("Browse##SoundFilepath")) {
            auto chosen = TossEngine::GetInstance().openFileDialog("*.ogg;*.mp3;");
            if (!chosen.empty()) {
                auto root = getProjectRoot();
                auto relPath = std::filesystem::relative(chosen, root);
                m_path = relPath.string();
                if (!m_path.empty()) AudioEngine::GetInstance().loadSound(*this);
            }
        }

        ImGui::EndTable();
    }
}

void Sound::onCreateLate()
{
    if (!m_path.empty()) AudioEngine::GetInstance().loadSound(*this);
}
