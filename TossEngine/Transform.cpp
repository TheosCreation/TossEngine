#include "Transform.h"
#include "GameObject.h"
#include "Scene.h"
#include "Prefab.h"
#include "ResourceManager.h"

Transform::Transform()
    : position(Vector3(0.0f, 0.0f, 0.0f)),
    rotation(Quaternion(1.0f, 0.0f, 0.0f, 0.0f)),
    scale(Vector3(1.0f, 1.0f, 1.0f)),
    localPosition(Vector3(0.0f, 0.0f, 0.0f)),
    localRotation(Quaternion(1.0f, 0.0f, 0.0f, 0.0f)),
    localScale(Vector3(1.0f, 1.0f, 1.0f)),
    parent(nullptr),
    gameObject(nullptr)
{
}

Transform::Transform(GameObject* attachedGameObject)
    : position(Vector3(0.0f, 0.0f, 0.0f)),
    rotation(Quaternion(1.0f, 0.0f, 0.0f, 0.0f)),
    scale(Vector3(1.0f, 1.0f, 1.0f)),
    localPosition(Vector3(0.0f, 0.0f, 0.0f)),
    localRotation(Quaternion(1.0f, 0.0f, 0.0f, 0.0f)),
    localScale(Vector3(1.0f, 1.0f, 1.0f)),
    parent(nullptr),
    gameObject(attachedGameObject)
{
}

Mat4 Transform::GetLocalMatrix() const
{
    // Create translation, rotation, and scale matrices from local components.
    Mat4 translationMatrix = localPosition.ToTranslation();
    Mat4 rotationMatrix = localRotation.ToMat4();
    Mat4 scaleMatrix = localScale.ToScale();
    return translationMatrix * rotationMatrix * scaleMatrix;
}

Mat4 Transform::GetMatrix() const
{
    if (parent != nullptr)
    {
        return parent->GetMatrix() * GetLocalMatrix();
    }
    return Mat4::Translate(position) * rotation.ToMat4() * Mat4::Scale(scale);
}

void Transform::SetMatrix(const Mat4& matrix)
{
    DecomposeMatrix(matrix.value, position, rotation, scale);

    if (parent) {
        glm::mat4 local = glm::inverse(parent->GetMatrix().value) * matrix.value;
        DecomposeMatrix(local, localPosition, localRotation, localScale);
    }
    else {
        // local mirrors world when no parent
        localPosition = position;
        localRotation = rotation;
        localScale = scale;
    }
}

void Transform::DecomposeMatrix(const glm::mat4& m, Vector3& pos, Quaternion& rot, Vector3& scl)
{
    pos = Vector3(m[3]);
    scl.x = glm::length(glm::vec3(m[0]));
    scl.y = glm::length(glm::vec3(m[1]));
    scl.z = glm::length(glm::vec3(m[2]));

    glm::mat4 rotMat(1.0f);
    rotMat[0] = glm::vec4(glm::normalize(glm::vec3(m[0])), 0.0f);
    rotMat[1] = glm::vec4(glm::normalize(glm::vec3(m[1])), 0.0f);
    rotMat[2] = glm::vec4(glm::normalize(glm::vec3(m[2])), 0.0f);
    rot = Quaternion(glm::quat_cast(rotMat));
}

void Transform::UpdateWorldTransform()
{
    constexpr float eps = FLT_EPSILON;

    const bool posChanged = !lastPosition.Equals(position, eps);
    const bool lposChanged = !lastLocalPosition.Equals(localPosition, eps);
    const bool sclChanged = !lastScale.Equals(scale, eps);
    const bool lsclChanged = !lastLocalScale.Equals(localScale, eps);
    const bool rotChanged = !lastRotation.Equals(rotation, eps);
    const bool lrotChanged = !lastLocalRotation.Equals(localRotation, eps);

    if (sclChanged && OnScaleChanged)       OnScaleChanged();
    if (lsclChanged && OnLocalScaleChanged)  OnLocalScaleChanged();
    if (rotChanged && OnRotationChanged)    OnRotationChanged();
    if (lrotChanged && OnLocalRotationChanged) OnLocalRotationChanged();
    if (posChanged && OnPositionChanged)    OnPositionChanged();
    if (lposChanged && OnLocalPositionChanged) OnLocalPositionChanged();

    const bool selfDirty =
        posChanged || lposChanged || sclChanged || lsclChanged || rotChanged || lrotChanged;

    // NEW: detect parent movement
    uint32_t parentVer = 0;
    bool parentDirty = false;
    if (parent) {
        parentVer = parent->worldVersion;
        parentDirty = (parentVer != lastAppliedParentVersion);
    }

    if (!selfDirty && !parentDirty) return;

    // Build world from correct source
    const Mat4 worldM = parent
        ? parent->GetMatrix() * GetLocalMatrix()
        : Mat4::Translate(position) * rotation.ToMat4() * Mat4::Scale(scale);

    SetMatrix(worldM);   // updates world; recomputes locals if parent exists

    // Snapshot AFTER mutation
    lastPosition = position;
    lastLocalPosition = localPosition;
    lastScale = scale;
    lastLocalScale = localScale;
    lastRotation = rotation;
    lastLocalRotation = localRotation;

    // Bump version and remember parent version used
    worldVersion++;
    lastAppliedParentVersion = parent ? parentVer : 0;
}


nlohmann::json Transform::serialize() const
{
    return {
        { "position", { position.x, position.y, position.z } },
        { "rotation", { rotation.x, rotation.y, rotation.z, rotation.w } },
        { "scale", { scale.x, scale.y, scale.z } },
        { "localPosition", { localPosition.x, localPosition.y, localPosition.z } },
        { "localRotation", { localRotation.x, localRotation.y, localRotation.z, localRotation.w } },
        { "localScale", { localScale.x, localScale.y, localScale.z } },
        { "parent", parent ? parent->gameObject->getId() : 0 }
    };
}

void Transform::deserialize(const nlohmann::json& data)
{
    if (data.contains("parent") && data["parent"] != 0)
    {
        size_t parentID = data["parent"].get<size_t>();

        if (gameObject != nullptr)
        {
            if (Transform* parent = LookupParentTransform(parentID))
            {
                SetParent(parent, false);
            }
        }
    }

    if (data.contains("position"))
        position = Vector3(data["position"][0], data["position"][1], data["position"][2]);

    if (data.contains("rotation"))
        rotation = Quaternion(data["rotation"][3], data["rotation"][0], data["rotation"][1], data["rotation"][2]);

    if (data.contains("scale"))
        scale = Vector3(data["scale"][0], data["scale"][1], data["scale"][2]);

    if (data.contains("localPosition"))
        localPosition = Vector3(data["localPosition"][0], data["localPosition"][1], data["localPosition"][2]);

    if (data.contains("localRotation"))
        localRotation = Quaternion(data["localRotation"][3], data["localRotation"][0], data["localRotation"][1], data["localRotation"][2]);

    if (data.contains("localScale"))
        localScale = Vector3(data["localScale"][0], data["localScale"][1], data["localScale"][2]);

    
}

Transform* Transform::LookupParentTransform(size_t parentID) const
{
    // First, try to find in the game object manager.
    if (Scene* gameObjectManager = gameObject->getScene())
    {
        auto& gameObjects = gameObjectManager->m_gameObjects;
        auto it = gameObjects.find(parentID);
        if (it != gameObjects.end())
        {
            return &it->second->m_transform;
        }
    }
    

    // If not found, try to find in the resource manager (for prefabs).
    auto prefabs = ResourceManager::GetInstance().getPrefabs();
    auto pit = std::find_if(prefabs.begin(), prefabs.end(),
        [parentID](const PrefabPtr& prefab) {
            return prefab->getId() == parentID;
        });

    if (pit != prefabs.end())
    {
        // If your Prefab class has a transform, return a pointer to it.
        Debug::Log("Found prefab as a parent");
        return &((*pit)->m_transform);
    }

    // Parent not found in either location.
    return nullptr;
}