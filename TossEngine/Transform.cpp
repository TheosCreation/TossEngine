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
    const glm::mat4& raw = matrix.value;

    // Extract translation (last column)
    position = Vector3(raw[3]);

    // Extract scale
    scale.x = Vector3(raw[0]).Length();
    scale.y = Vector3(raw[1]).Length();
    scale.z = Vector3(raw[2]).Length();

    // Remove scale from matrix to isolate rotation
    glm::mat4 rotMatrix = glm::mat4(1.0f);
    rotMatrix[0] = glm::vec4(glm::normalize(glm::vec3(raw[0])), 0.0f);
    rotMatrix[1] = glm::vec4(glm::normalize(glm::vec3(raw[1])), 0.0f);
    rotMatrix[2] = glm::vec4(glm::normalize(glm::vec3(raw[2])), 0.0f);
    rotMatrix[3] = glm::vec4(0, 0, 0, 1);

    rotation = Quaternion(glm::quat_cast(rotMatrix));
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
                Debug::Log("Parent Set");
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
