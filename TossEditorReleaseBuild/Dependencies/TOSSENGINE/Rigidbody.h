#pragma once
#include "Component.h"
#include <reactphysics3d/reactphysics3d.h>

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
            static Vector3 boxSize(1.0f, 1.0f, 1.0f);
            static float sphereRadius = 0.5f;

            // Box Collider settings
            if (ImGui::Button("Add Box Collider"))
            {
                SetBoxCollider(boxSize);
            }
            ImGui::SameLine();
            ImGui::InputFloat3("Box Size", &boxSize.x);

            // Sphere Collider settings
            if (ImGui::Button("Add Sphere Collider"))
            {
                SetSphereCollider(sphereRadius);
            }
            ImGui::SameLine();
            ImGui::InputFloat("Sphere Radius", &sphereRadius, 0.1f, 0.5f, "%.2f");
        }
    }

    void onCreate() override;
    void onFixedUpdate(float fixedDeltaTime) override;

    void SetBodyType(BodyType type);
    void SetMass(float mass);
    void SetUseGravity(bool useGravity);
    void SetBoxCollider(const Vector3& size);
    void SetSphereCollider(float radius);

private:
    rp3d::RigidBody* m_Body = nullptr;
    rp3d::Collider* m_Collider = nullptr;
    BodyType m_BodyType = BodyType::Dynamic; // Default to Dynamic
};

REGISTER_COMPONENT(Rigidbody);