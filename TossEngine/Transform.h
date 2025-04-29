/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : Transform.h
Description : Component that defines position, rotation, and scale in world and local space.
              Supports hierarchical parenting and matrix generation for rendering and physics.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "TossEngineAPI.h"
#include "Math.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Mat4.h"
#include "Debug.h"
#include <nlohmann/json.hpp>

class GameObject;

/**
 * @struct Transform
 * @brief Defines an object's position, rotation, and scale in 3D space. Supports hierarchical transforms.
 */
struct TOSSENGINE_API Transform
{
    // --- World-space transform ---
    Vector3 position = Vector3();         //!< World position
    Quaternion rotation = Quaternion();  //!< World rotation
    Vector3 scale = Vector3();           //!< World scale

    // --- Local-space transform ---
    Vector3 localPosition = Vector3();   //!< Local position relative to parent
    Quaternion localRotation = Quaternion(); //!< Local rotation relative to parent
    Vector3 localScale = Vector3();      //!< Local scale relative to parent

    // --- Hierarchy ---
    Transform* parent = nullptr;         //!< Pointer to parent transform
    std::vector<Transform*> children;    //!< List of child transforms

    GameObject* gameObject = nullptr;    //!< Owning GameObject

    // --- Constructors ---

    /**
     * @brief Default constructor.
     */
    Transform();

    /**
     * @brief Constructor that attaches this transform to a GameObject.
     * @param attachedGameObject The GameObject to attach to.
     */
    Transform(GameObject* attachedGameObject);

    // --- Matrix Operations ---

    /**
     * @brief Returns the local model matrix.
     */
    Mat4 GetLocalMatrix() const;

    /**
     * @brief Returns the world model matrix, accounting for parent hierarchy.
     */
    Mat4 GetMatrix() const;

    /**
     * @brief Applies a matrix to this transform, decomposing it into position, rotation, scale.
     * @param matrix The matrix to decompose.
     */
    void SetMatrix(const Mat4& matrix);

    /**
     * @brief Updates the world-space transform based on local and parent data.
     */
    void UpdateWorldTransform();

    // --- Serialization ---

    /**
     * @brief Serializes the transform to JSON.
     */
    nlohmann::json serialize() const;

    /**
     * @brief Deserializes the transform from JSON.
     */
    void deserialize(const nlohmann::json& data);

    /**
     * @brief Searches the parent hierarchy for a transform matching the given GameObject ID.
     * @param parentID The ID to search for.
     * @return A pointer to the matching parent transform, or nullptr if not found.
     */
    Transform* LookupParentTransform(size_t parentID) const;

    // --- Getters and Setters ---

    Vector3 ToEulerAngles() const { return rotation.ToEulerAngles(); }

    void SetPosition(const Vector3& newPosition) { position = newPosition; }

    void SetRotation(const Quaternion& newRotation) { rotation = newRotation; }

    void SetScale(const Vector3& newScale) { scale = newScale; }

    void SetLocalScale(const Vector3& newScale);

    Vector3 GetLocalScale() const { return localScale; }

    Vector3 GetWorldScale() const { return scale; /* NOTE: recursive scale not yet implemented */ }

    // --- Transformation Operations ---

    void Translate(const Vector3& translation) { position += translation; }

    void Rotate(const Quaternion& deltaRotation)
    {
        rotation = deltaRotation * rotation;
        rotation.Normalize();
    }

    void Scale(const Vector3& scaleFactor) { scale *= scaleFactor; }

    // --- Directional Vectors ---

    Vector3 GetForward() const { return rotation * Vector3(0.0f, 0.0f, -1.0f); }

    Vector3 GetRight() const { return rotation * Vector3(1.0f, 0.0f, 0.0f); }

    Vector3 GetUp() const { return rotation * Vector3(0.0f, 1.0f, 0.0f); }

    // --- Parenting ---

    /**
     * @brief Sets a new parent and optionally keeps world-space transform.
     * @param newParent The new parent transform.
     * @param keepWorldOffset If true, preserve world position/rotation/scale.
     */
    void SetParent(Transform* newParent, bool keepWorldOffset = true)
    {
        Mat4 worldMatrix = GetMatrix();

        if (parent)
            parent->RemoveChild(this);

        parent = newParent;

        if (parent)
        {
            if (keepWorldOffset)
            {
                Mat4 parentInverse = parent->GetMatrix().Inverse();
                Mat4 localMatrix = parentInverse * worldMatrix;

                localPosition = Vector3::ExtractTranslation(localMatrix);
                localScale = Vector3::ExtractScale(localMatrix);
                localRotation = Quaternion::ExtractRotation(localMatrix);
            }
            else
            {
                localPosition = Vector3(0.0f);
                localRotation = Quaternion::Identity();
                localScale = Vector3(1.0f);
            }

            parent->children.push_back(this);
        }
    }

private:
    /**
     * @brief Removes a child transform from this transform.
     */
    void RemoveChild(Transform* child)
    {
        std::erase(children, child);
    }
};

// --- JSON Serialization for Transform ---
inline void to_json(json& j, Transform const& transform) {
    j = transform.serialize();
}

inline void from_json(json const& j, Transform& transform) {
    transform = Transform(); // Reset state
    transform.deserialize(j);
}