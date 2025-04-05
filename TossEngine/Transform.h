#pragma once
#include "TossEngineAPI.h"
#include "Math.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Mat4.h"
#include <nlohmann\json.hpp>

class GameObject;

// Structure representing the transformation of an object in 3D space
struct TOSSENGINE_API Transform
{
    Vector3 position;         // World position
    Quaternion rotation;      // World rotation
    Vector3 scale;            // World scale

    Vector3 localPosition;    // Local position relative to parent
    Quaternion localRotation; // Local rotation relative to parent
    Vector3 localScale;       // Local scale relative to parent

    Transform* parent;                // Pointer to parent transform (nullptr if root)
    std::vector<Transform*> children; // List of pointers to child transforms

    GameObject* gameObject;   // Pointer to the GameObject this Transform is attached to

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

    Vector3 ToEulerAngles()
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

    void SetParent(Transform* newParent)
    {
        if (parent)
        {
            parent->RemoveChild(this);
        }

        parent = newParent;
        if (parent)
        {
            localPosition = parent->GetMatrix().Inverse() * position;
            parent->children.push_back(this);
        }
    }

private:
    void RemoveChild(Transform* child)
    {
        children.erase(std::remove(children.begin(), children.end(), child), children.end());
    }
};