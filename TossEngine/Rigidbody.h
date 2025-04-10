#pragma once
#include "Component.h"
#include "Physics.h"

using BodyType = rp3d::BodyType;

class Collider;

class TOSSENGINE_API Rigidbody : public Component
{
public:
    Rigidbody() = default;
    ~Rigidbody();

    json serialize() const override;
    void deserialize(const json& data) override;

    void OnInspectorGUI() override;
    void OnGameObjectSelected() override;
    void OnGameObjectDeSelected() override;

    void onCreate() override;
    void onLateCreate() override;
    void onStart() override;
    void onUpdate() override;
    void onUpdateInternal() override;
    void UpdateBodyTransform();

    void SetBodyType(BodyType type);
    void SetMass(float mass);
    void SetUseGravity(bool useGravity);

    rp3d::RigidBody* GetBody();
    Collider* GetCollider();

    Vector3 GetLinearVelocity() const;
    void SetLinearVelocity(const Vector3& velocity);

    Vector3 GetAngularVelocity() const;
    void SetAngularVelocity(const Vector3& velocity);

    void AddForce(const Vector3& force);
    void AddTorque(const Vector3& torque);

    void SetPositionConstraints(bool lockX, bool lockY, bool lockZ);
    void SetRotationConstraints(bool lockX, bool lockY, bool lockZ);

    void OnCollisionEnter(Rigidbody* collidedRb);
    void OnCollisionExit(Rigidbody* collidedRb);

private:
    rp3d::RigidBody* m_Body = nullptr;
    Collider* m_Collider = nullptr; // Reference to separate Collider component
    BodyType m_BodyType = BodyType::DYNAMIC;
    bool m_UseGravity = true;


    void UpdatePositionConstraints();
    void UpdateRotationConstraints();

    std::array<bool, 3> positionAxisLocks = { false, false, false };
    std::array<bool, 3> rotationAxisLocks = { false, false, false };

};

REGISTER_COMPONENT(Rigidbody);