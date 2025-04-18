#pragma once
#include "TossEngineAPI.h"
#include "Math.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Mat4.h"
#include "Debug.h"
#include <nlohmann\json.hpp>

class GameObject;

// Structure representing the transformation of an object in 3D space
struct TOSSENGINE_API Transform
{
    Vector3 position = Vector3();         // World position
    Quaternion rotation = Quaternion();      // World rotation
    Vector3 scale = Vector3();            // World scale

    Vector3 localPosition = Vector3();    // Local position relative to parent
    Quaternion localRotation = Quaternion(); // Local rotation relative to parent

    Transform* parent = nullptr;                // Pointer to parent transform (nullptr if root)
    std::vector<Transform*> children; // List of pointers to child transforms

    GameObject* gameObject = nullptr;   // Pointer to the GameObject this Transform is attached to

    // Default Constructor for none attached transforms and just data
    Transform();

    // Constructor initializes defaults and attaches GameObject
    Transform(GameObject* attachedGameObject);

    Mat4 GetLocalMatrix() const;

    Mat4 GetMatrix() const;

    void SetMatrix(const Mat4& matrix);

    void UpdateWorldTransform();

    nlohmann::json serialize() const;

    void deserialize(const nlohmann::json& data);

    Transform* LookupParentTransform(size_t parentID) const;

    Vector3 ToEulerAngles() const
    {
        return rotation.ToEulerAngles();
    }

    void SetPosition(const Vector3& newPosition)
    {
        position = newPosition;
    }

    void SetRotation(const Quaternion& newRotation)
    {
        rotation = newRotation;
    }

    Vector3 GetLocalScale() const
    {
        return localScale;
    }

    void SetLocalScale(const Vector3& newScale);
    
    void SetScale(const Vector3& newScale)
    {
        scale = newScale;
    }

    void Translate(const Vector3& translation)
    {
        position += translation;
    }

    void Rotate(const Quaternion& deltaRotation)
    {
        rotation = deltaRotation * rotation;
        rotation.Normalize();
    }

    void Scale(const Vector3& scaleFactor)
    {
        scale *= scaleFactor;
    }
    
    Vector3 GetWorldScale() const {
        return scale; //need to implement when doing parenting
    }

    Vector3 GetForward() const
    {
        return rotation * Vector3(0.0f, 0.0f, -1.0f);
    }

    Vector3 GetRight() const
    {
        return rotation * Vector3(1.0f, 0.0f, 0.0f);
    }

    Vector3 GetUp() const
    {
        return rotation * Vector3(0.0f, 1.0f, 0.0f);
    }

    void SetParent(Transform* newParent, bool keepWorldOffset = true)
    {
        Mat4 worldMatrix = GetMatrix();

        if (parent)
        {
            parent->RemoveChild(this);
        }

        parent = newParent;
        if (parent)
        {
            if (keepWorldOffset)
            {
                Mat4 parentWorld = parent->GetMatrix();

                Mat4 parentInverse = parentWorld.Inverse();

                Mat4 localMatrix = parentInverse * worldMatrix;

                localPosition = Vector3::ExtractTranslation(localMatrix);
                localScale = Vector3::ExtractScale(localMatrix);
                localRotation = Quaternion::ExtractRotation(localMatrix);
            }
            else
            {
                localPosition = Vector3(0.0f, 0.0f, 0.0f);
                localRotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
                localScale = Vector3(1.0f, 1.0f, 1.0f);
            }
            parent->children.push_back(this);
        }
    }
    
private:
    void RemoveChild(Transform* child)
    {
        std::erase(children, child);
    }

    Vector3 localScale = Vector3();       // Local scale relative to parent
};