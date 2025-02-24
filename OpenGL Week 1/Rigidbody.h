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
    Rigidbody();
    ~Rigidbody();

    virtual void onCreate() override;
    virtual void onFixedUpdate(float fixedDeltaTime) override;

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