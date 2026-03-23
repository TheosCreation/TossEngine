/***
DeviousDevs
Auckland
New Zealand
(c) 2026 DeviousDevs
File Name : Animation.h
Description : Animation resource used to animation rigs/skinned meshes
Author : Theo Morris
Mail : theo.morris@outlook.co.nz
***/

#pragma once
#include "../Resource.h"
#include "../Mesh.h"

/**
 * @class Animation
 * @brief Resource wrapper for animations.
 */
class TOSSENGINE_API Animation : public Resource
{
public:
    Animation(const std::string& uid, ResourceManager* mgr);

    void OnInspectorGUI() override;

    MeshPtr GetMesh() const;
    const std::string& GetClipName() const;

private:
    MeshPtr m_mesh;
    std::string m_clipName;

    SERIALIZABLE_MEMBERS(m_path, m_mesh, m_clipName)
};
REGISTER_RESOURCE(Animation, ".anim", ".anim")

inline void to_json(json& j, AnimationPtr const& animation)
{
    if (animation)
    {
        j = json{ { "id", animation->getUniqueID() } };
    }
    else
    {
        j = nullptr;
    }
}

inline void from_json(json const& j, AnimationPtr& animation)
{
    if (j.contains("id"))
    {
        animation = ResourceManager::GetInstance().get<Animation>(j["id"].get<string>());
    }
}
