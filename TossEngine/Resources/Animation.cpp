#include "Animation.h"

Animation::Animation(const std::string& uid, ResourceManager* mgr)
    : Resource(uid, mgr)
{
}

void Animation::OnInspectorGUI()
{
    ImGui::Text("Path: %s", getPath().c_str());
    ImGui::Separator();

    ResourceAssignableField(m_mesh, "Mesh");

    char clipBuffer[256];
    memset(clipBuffer, 0, sizeof(clipBuffer));
    strncpy_s(clipBuffer, m_clipName.c_str(), sizeof(clipBuffer) - 1);

    if (ImGui::InputText("Clip Name", clipBuffer, sizeof(clipBuffer)))
    {
        m_clipName = clipBuffer;
    }

    if (m_mesh != nullptr)
    {
        const std::vector<AnimationClipData>& animationClips = m_mesh->GetAnimationClips();
        ImGui::Text("Mesh Clip Count: %d", static_cast<int>(animationClips.size()));

        for (const AnimationClipData& clip : animationClips)
        {
            ImGui::BulletText("%s", clip.name.c_str());
        }
    }
}

MeshPtr Animation::GetMesh() const
{
    return m_mesh;
}

const std::string& Animation::GetClipName() const
{
    return m_clipName;
}
