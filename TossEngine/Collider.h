#pragma once
#include "Component.h"
#include "Physics.h"

class Rigidbody;

class TOSSENGINE_API Collider : public Component {
public:
    Collider() = default;
    ~Collider() = default;

    json serialize() const override;
    void deserialize(const json& data) override;

    virtual void OnInspectorGUI() override;

    void onStart() override;
    void onCreate() override;

    void SetBoxCollider(const Vector3& size);
    void SetSphereCollider(float radius);
    void SetCapsuleCollider(float radius, float height);
    void SetTrigger(bool trigger);
    bool GetTrigger();

    rp3d::CollisionShape* GetColliderShape() const { return m_Shape; }
    rp3d::CollisionShapeType GetColliderType() const;
    PhysicsMaterialPtr GetPhysicsMaterial() const;

private:
    rp3d::CollisionShape* m_Shape = nullptr;
    Vector3 m_boxColliderSize = Vector3(1.0f);
    float m_radius = 1.0f;
    float m_height = 1.0f;
    bool m_isTrigger = false;

    Rigidbody* m_Rigidbody = nullptr;
    PhysicsMaterialPtr m_physicsMaterial = nullptr;
};

REGISTER_COMPONENT(Collider);