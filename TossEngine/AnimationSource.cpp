#include "AnimationSource.h"

#include "Resources/Animation.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "Time.h"

static int FindPositionKeyIndex(const std::vector<PositionKey>& keys, float time)
{
    if (keys.size() < 2)
    {
        return 0;
    }

    for (int keyIndex = 0; keyIndex < static_cast<int>(keys.size()) - 1; keyIndex++)
    {
        if (time < keys[keyIndex + 1].time)
        {
            return keyIndex;
        }
    }

    return static_cast<int>(keys.size()) - 2;
}

static int FindRotationKeyIndex(const std::vector<RotationKey>& keys, float time)
{
    if (keys.size() < 2)
    {
        return 0;
    }

    for (int keyIndex = 0; keyIndex < static_cast<int>(keys.size()) - 1; keyIndex++)
    {
        if (time < keys[keyIndex + 1].time)
        {
            return keyIndex;
        }
    }

    return static_cast<int>(keys.size()) - 2;
}

static int FindScaleKeyIndex(const std::vector<ScaleKey>& keys, float time)
{
    if (keys.size() < 2)
    {
        return 0;
    }

    for (int keyIndex = 0; keyIndex < static_cast<int>(keys.size()) - 1; keyIndex++)
    {
        if (time < keys[keyIndex + 1].time)
        {
            return keyIndex;
        }
    }

    return static_cast<int>(keys.size()) - 2;
}

static float CalculateInterpolationFactor(float startTime, float endTime, float currentTime)
{
    float deltaTime = endTime - startTime;
    if (deltaTime <= 0.0f)
    {
        return 0.0f;
    }

    float factor = (currentTime - startTime) / deltaTime;

    if (factor < 0.0f)
    {
        factor = 0.0f;
    }

    if (factor > 1.0f)
    {
        factor = 1.0f;
    }

    return factor;
}

static Vector3 SamplePosition(const BoneTrack& track, float time)
{
    if (track.positions.empty())
    {
        return Vector3(0.0f);
    }

    if (track.positions.size() == 1)
    {
        return track.positions[0].value;
    }

    int keyIndex = FindPositionKeyIndex(track.positions, time);
    const PositionKey& currentKey = track.positions[keyIndex];
    const PositionKey& nextKey = track.positions[keyIndex + 1];

    float factor = CalculateInterpolationFactor(currentKey.time, nextKey.time, time);
    return Vector3::Lerp(currentKey.value, nextKey.value, factor);
}

static Quaternion SampleRotation(const BoneTrack& track, float time)
{
    if (track.rotations.empty())
    {
        return Quaternion();
    }

    if (track.rotations.size() == 1)
    {
        return track.rotations[0].value;
    }

    int keyIndex = FindRotationKeyIndex(track.rotations, time);
    const RotationKey& currentKey = track.rotations[keyIndex];
    const RotationKey& nextKey = track.rotations[keyIndex + 1];

    float factor = CalculateInterpolationFactor(currentKey.time, nextKey.time, time);
    return Quaternion::Slerp(currentKey.value, nextKey.value, factor);
}

static Vector3 SampleScale(const BoneTrack& track, float time)
{
    if (track.scales.empty())
    {
        return Vector3(1.0f);
    }

    if (track.scales.size() == 1)
    {
        return track.scales[0].value;
    }

    int keyIndex = FindScaleKeyIndex(track.scales, time);
    const ScaleKey& currentKey = track.scales[keyIndex];
    const ScaleKey& nextKey = track.scales[keyIndex + 1];

    float factor = CalculateInterpolationFactor(currentKey.time, nextKey.time, time);
    return Vector3::Lerp(currentKey.value, nextKey.value, factor);
}

void AnimationSource::onCreate()
{
    Component::onCreate();
}

void AnimationSource::onStart()
{
    if (m_playOnStart) Play(m_clipName, m_loop);
}

void AnimationSource::OnInspectorGUI()
{
    Component::OnInspectorGUI();

    ResourceAssignableField(m_animation, "Animation");

    char clipNameBuffer[256] = {};
    strncpy_s(clipNameBuffer, m_clipName.c_str(), sizeof(clipNameBuffer) - 1);

    if (ImGui::InputText("Clip Name", clipNameBuffer, sizeof(clipNameBuffer)))
    {
        m_clipName = clipNameBuffer;
        m_currentTime = 0.0f;
    }

    ImGui::Checkbox("Playing", &m_isPlaying);
    ImGui::Checkbox("PlayOnStart", &m_playOnStart);
    ImGui::Checkbox("Loop", &m_loop);
    ImGui::DragFloat("Current Time", &m_currentTime, 0.01f, 0.0f);
}

void AnimationSource::Play(const std::string& clipName, bool loop)
{
    m_clipName = clipName;
    m_loop = loop;
    m_currentTime = 0.0f;
    m_isPlaying = true;

    ApplyCurrentPose();
}

void AnimationSource::Stop()
{
    m_isPlaying = false;
    m_currentTime = 0.0f;

    MeshRenderer* meshRenderer = m_owner->getComponent<MeshRenderer>();
    if (meshRenderer == nullptr)
    {
        return;
    }

    meshRenderer->ResetBonesToBindPose();
}

const AnimationClipData* AnimationSource::GetCurrentClip() const
{
    MeshRenderer* meshRenderer = m_owner->getComponent<MeshRenderer>();
    if (meshRenderer == nullptr)
    {
        return nullptr;
    }

    MeshPtr mesh = meshRenderer->GetMesh();
    if (mesh == nullptr)
    {
        return nullptr;
    }

    if (m_animation != nullptr && m_animation->GetMesh() != nullptr)
    {
        return m_animation->GetMesh()->GetAnimationClip(m_animation->GetClipName());
    }

    return mesh->GetAnimationClip(m_clipName);
}

void AnimationSource::ApplyCurrentPose()
{
    MeshRenderer* meshRenderer = m_owner->getComponent<MeshRenderer>();
    if (meshRenderer == nullptr)
    {
        return;
    }

    MeshPtr mesh = meshRenderer->GetMesh();
    if (mesh == nullptr)
    {
        return;
    }

    const AnimationClipData* clip = GetCurrentClip();
    if (clip == nullptr)
    {
        return;
    }

    meshRenderer->ResetBonesToBindPose();

    for (const BoneTrack& track : clip->tracks)
    {
        GameObjectPtr boneObject = meshRenderer->GetBoneObjectByName(track.boneName);
        if (boneObject == nullptr)
        {
            continue;
        }

        Vector3 localPosition = SamplePosition(track, m_currentTime);
        Quaternion localRotation = SampleRotation(track, m_currentTime);
        Vector3 localScale = SampleScale(track, m_currentTime);

        boneObject->m_transform.localPosition = localPosition;
        boneObject->m_transform.localRotation = localRotation;
        boneObject->m_transform.localScale = localScale;
    }
}

void AnimationSource::onUpdate()
{
    if (!m_isPlaying)
    {
        return;
    }

    MeshRenderer* meshRenderer = m_owner->getComponent<MeshRenderer>();
    if (meshRenderer == nullptr)
    {
        return;
    }

    MeshPtr mesh = meshRenderer->GetMesh();
    if (mesh == nullptr || !mesh->IsSkinned())
    {
        return;
    }

    const AnimationClipData* clip = GetCurrentClip();
    if (clip == nullptr)
    {
        return;
    }

    float ticksPerSecond = clip->ticksPerSecond > 0.0f ? clip->ticksPerSecond : 25.0f;
    m_currentTime += Time::DeltaTime * ticksPerSecond;

    if (m_loop && clip->duration > 0.0f)
    {
        while (m_currentTime >= clip->duration)
        {
            m_currentTime -= clip->duration;
        }
    }
    else if (m_currentTime > clip->duration)
    {
        m_currentTime = clip->duration;
        m_isPlaying = false;
    }

    ApplyCurrentPose();
}