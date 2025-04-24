#include "Transform.h"
#include "GameObject.h"
#include "GameObjectManager.h"
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
    if (parent)
        return parent->GetMatrix() * GetLocalMatrix();
    else
        return GetLocalMatrix();// Create translation, rotation, and scale matrices from local components.
}

void Transform::SetMatrix(const Mat4& matrix)
{
    const glm::mat4 world = matrix.value;

    glm::mat4 parentWorld = parent
        ? parent->GetMatrix().value
        : glm::mat4(1.0f);
    glm::mat4 localMat = glm::inverse(parentWorld) * world;

    localPosition = Vector3(localMat[3]);   // column 3 is translation

    Vector3 previousScale = localScale;
    localScale.x = glm::length(glm::vec3(localMat[0]));
    localScale.y = glm::length(glm::vec3(localMat[1]));
    localScale.z = glm::length(glm::vec3(localMat[2]));
    gameObject->onLocalScaleChanged(previousScale);

    glm::mat4 rotMat = glm::mat4(1.0f);
    rotMat[0] = glm::vec4(glm::normalize(glm::vec3(localMat[0])), 0.0f);
    rotMat[1] = glm::vec4(glm::normalize(glm::vec3(localMat[1])), 0.0f);
    rotMat[2] = glm::vec4(glm::normalize(glm::vec3(localMat[2])), 0.0f);

    localRotation = Quaternion(glm::quat_cast(rotMat));

    position = Vector3(world[3]);
    scale.x = glm::length(glm::vec3(world[0]));
    scale.y = glm::length(glm::vec3(world[1]));
    scale.z = glm::length(glm::vec3(world[2]));

    glm::mat4 worldRot = glm::mat4(1.0f);
    worldRot[0] = glm::vec4(glm::normalize(glm::vec3(world[0])), 0.0f);
    worldRot[1] = glm::vec4(glm::normalize(glm::vec3(world[1])), 0.0f);
    worldRot[2] = glm::vec4(glm::normalize(glm::vec3(world[2])), 0.0f);
    rotation = Quaternion(glm::quat_cast(worldRot));
}

void Transform::UpdateWorldTransform()
{
    if (parent)
    {
        Mat4 worldMatrix = parent->GetMatrix() * GetLocalMatrix(); 
        SetMatrix(worldMatrix);
    }
    else
    {
        position = localPosition;
        rotation = localRotation;
        scale = localScale;
    }

    // Recursively update children
    for (Transform* child : children)
    {
        child->UpdateWorldTransform();
    }
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
    if (GameObjectManager* gameObjectManager = gameObject->getGameObjectManager())
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

void Transform::SetLocalScale(const Vector3& newScale)
{
    Vector3 previousScale = localScale;
    localScale = newScale;
    if (gameObject)
    {
        gameObject->onLocalScaleChanged(previousScale);
    }
}
