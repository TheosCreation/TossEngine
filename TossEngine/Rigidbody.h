#pragma once
#include "Component.h"
#include <reactphysics3d/reactphysics3d.h>

enum class BodyType {
    Static,
    Kinematic,
    Dynamic
};

class Rigidbody : public Component {
public:
    Rigidbody() = default;
    ~Rigidbody();

    json serialize() const override;

    void deserialize(const json& data) override;

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