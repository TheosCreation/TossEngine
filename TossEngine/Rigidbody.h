#pragma once
#include "Component.h"
#include "Physics.h"

enum class BodyType {
    Static,
    Kinematic,
    Dynamic
};

class TOSSENGINE_API Rigidbody : public Component {
public:
    Rigidbody() = default;
    ~Rigidbody();

    json serialize() const override;

    void deserialize(const json& data) override;

    virtual void OnInspectorGUI() override
    {
        ImGui::Text("Rigidbody Inspector - ID: %p", this);
        ImGui::Separator();

        // Body Type selector
        const char* bodyTypes[] = { "Static", "Kinematic", "Dynamic" };
        int currentBodyType = static_cast<int>(m_BodyType);
        if (ImGui::Combo("Body Type", &currentBodyType, bodyTypes, IM_ARRAYSIZE(bodyTypes)))
        {
            SetBodyType(static_cast<BodyType>(currentBodyType));
        }

        // Mass input
        float mass = (m_Body) ? m_Body->getMass() : 0.0f;
        if (ImGui::InputFloat("Mass", &mass, 0.1f, 1.0f, "%.2f"))
        {
            SetMass(mass);
        }

        // Gravity toggle
        bool useGravity = (m_Body) ? m_Body->isGravityEnabled() : true;
        if (ImGui::Checkbox("Use Gravity", &useGravity))
        {
            SetUseGravity(useGravity);
        }

        ImGui::Separator();

        // Collider section
        if (ImGui::CollapsingHeader("Collider Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (m_Collider)
            {
                auto shapeType = m_Collider->getCollisionShape()->getType();

                if (shapeType == rp3d::CollisionShapeType::CONVEX_POLYHEDRON)
                {
                    ImGui::Text("Box Collider:");
                    Vector3 size = m_boxColliderSize;
                    if (ImGui::InputFloat3("Size", size.Data()))
                    {
                        SetBoxCollider(size); // Update size
                    }
                }
                else if (shapeType == rp3d::CollisionShapeType::SPHERE)
                {
                    ImGui::Text("Sphere Collider:");
                    float radius = m_radius;
                    if (ImGui::InputFloat("Radius", &radius, 0.1f, 0.5f, "%.2f"))
                    {
                        SetSphereCollider(radius); // Update radius
                    }
                }
                else if (shapeType == rp3d::CollisionShapeType::CAPSULE)
                {
                    ImGui::Text("Capsule Collider:");
                    float radius = m_radius;
                    float height = m_height;
                    bool somethingupdated = false;
                    if (ImGui::InputFloat("Radius", &radius, 0.1f, 0.5f, "%.2f"))
                    {
                        somethingupdated = true;
                    }
                    if (ImGui::InputFloat("Height", &height, 0.1f, 0.5f, "%.2f"))
                    {
                        somethingupdated = true;
                    }
                    if (somethingupdated)
                    {
                        SetCapsuleCollider(radius, height); // Update radius and height
                    }
                }
                else
                {
                    ImGui::Text("Unknown Collider Type");
                }

                if (ImGui::Button("Remove Collider"))
                {
                    RemoveCollider();
                }
            }
            else
            {
                ImGui::Text("No Collider Found");

                static Vector3 newBoxSize(1.0f, 1.0f, 1.0f);
                ImGui::InputFloat3("New Box Size", newBoxSize.Data());
                if (ImGui::Button("Add Box Collider"))
                {
                    SetBoxCollider(newBoxSize);
                }

                static float newSphereRadius = 0.5f;
                ImGui::InputFloat("New Sphere Radius", &newSphereRadius, 0.1f, 0.5f, "%.2f");
                if (ImGui::Button("Add Sphere Collider"))
                {
                    SetSphereCollider(newSphereRadius);
                }

                static float newCapsuleRadius = 0.5f;
                static float newCapsuleHeight = 1.0f;
                ImGui::InputFloat("New Capsule Radius", &newCapsuleRadius, 0.1f, 0.5f, "%.2f");
                ImGui::InputFloat("New Capsule Height", &newCapsuleHeight, 0.1f, 0.5f, "%.2f");
                if (ImGui::Button("Add Capsule Collider"))
                {
                    SetCapsuleCollider(newCapsuleRadius, newCapsuleHeight);
                }
            }
        }

        if (ImGui::CollapsingHeader("Constraints", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Position Locks");
            ImGui::Checkbox("X##pos", &positionAxisLocks[0]);
            ImGui::SameLine();
            ImGui::Checkbox("Y##pos", &positionAxisLocks[1]);
            ImGui::SameLine();
            ImGui::Checkbox("Z##pos", &positionAxisLocks[2]);

            ImGui::Text("Rotation Locks");
            ImGui::Checkbox("X##rot", &rotationAxisLocks[0]);
            ImGui::SameLine();
            ImGui::Checkbox("Y##rot", &rotationAxisLocks[1]);
            ImGui::SameLine();
            ImGui::Checkbox("Z##rot", &rotationAxisLocks[2]);
        }
    }

    void onCreate() override;
    void onStart() override; // we dont allow for runtime scaling yet for the colliders
    void onUpdate(float deltaTime) override;

    void SetBodyType(BodyType type);
    void SetMass(float mass);
    void SetUseGravity(bool useGravity);
    void SetBoxCollider(const Vector3& size);
    void SetSphereCollider(float radius);
    void SetCapsuleCollider(float radius, float height);
    void RemoveCollider();

    Vector3 GetLinearVelocity() const {
        return m_Body ? Vector3(m_Body->getLinearVelocity()) : Vector3(0.0f);
    }

    void SetLinearVelocity(const Vector3& velocity) {
        if (m_Body)
            m_Body->setLinearVelocity(static_cast<rp3d::Vector3>(velocity));
    }

    Vector3 GetAngularVelocity() const {
        return m_Body ? Vector3(m_Body->getAngularVelocity()) : Vector3(0.0f);
    }

    void SetAngularVelocity(const Vector3& velocity) {
        if (m_Body)
            m_Body->setAngularVelocity(static_cast<rp3d::Vector3>(velocity));
    }
    
    void AddForce(const Vector3& force) {
        if (m_Body)
            m_Body->applyWorldForceAtCenterOfMass(static_cast<rp3d::Vector3>(force));
    }

    void AddTorque(const Vector3& torque) {
        if (m_Body)
            m_Body->applyWorldTorque(static_cast<rp3d::Vector3>(torque));
    }
    
    void SetPositionConstraints(bool lockX, bool lockY, bool lockZ) {
        if (m_Body)
            m_Body->setLinearLockAxisFactor(rp3d::Vector3(!lockX, !lockY, !lockZ));
    }

    void SetRotationConstraints(bool lockX, bool lockY, bool lockZ) {
        if (m_Body)
            m_Body->setAngularLockAxisFactor(rp3d::Vector3(!lockX, !lockY, !lockZ));
    }


private:
    rp3d::RigidBody* m_Body = nullptr;
    rp3d::Collider* m_Collider = nullptr;
    BodyType m_BodyType = BodyType::Dynamic; // Default to Dynamic

    std::array<bool, 3> positionAxisLocks = { false, false, false };
    std::array<bool, 3> rotationAxisLocks = { false, false, false };

    float m_radius = 1.0f;
    float m_height = 1.0f;
    Vector3 m_boxColliderSize = Vector3(1.0f);
};

REGISTER_COMPONENT(Rigidbody);