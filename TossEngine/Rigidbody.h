#pragma once
#include "Component.h"
#include "Physics.h"

using BodyType = rp3d::BodyType;

class Collider;

class TOSSENGINE_API Rigidbody : public Component
{
public:
    json serialize() const override;
    void deserialize(const json& data) override;

    void OnInspectorGUI() override;
    void OnGameObjectSelected() override;
    void OnGameObjectDeSelected() override;

    void onCreate() override;
    void onCreateLate() override;
    void onStart() override;
    void onUpdate() override;
    void onUpdateInternal() override;
    void UpdateBodyTransform() const;
    void onDestroy() override;

    void SetBodyType(BodyType type);
    void SetMass(float mass) const;
    void SetUseGravity(bool useGravity);

    rp3d::RigidBody* GetBody() const;
    Collider* GetCollider() const;

    Vector3 GetLinearVelocity() const;
    void SetLinearVelocity(const Vector3& velocity);

    Vector3 GetAngularVelocity() const;
    void SetAngularVelocity(const Vector3& velocity) const;

    /**
     * Adds force to the center of mass of the rigidbody
     * Limitation of the force needing to be larger than 0.1f
     * @param force 
     */
    void AddForce(const Vector3& force) const;

    /**
     * Adds torque to the center of mass of the rigidbody
     * Limitation of the torque needing to be larger than 0.1f
     * @param torque 
     */
    void AddTorque(const Vector3& torque) const;

    void SetPositionConstraints(bool lockX, bool lockY, bool lockZ);
    void SetRotationConstraints(bool lockX, bool lockY, bool lockZ);


private:
    rp3d::RigidBody* m_Body = nullptr;
    Collider* m_Collider = nullptr; // Reference to separate Collider component
    BodyType m_BodyType = BodyType::DYNAMIC;
    bool m_UseGravity = true;
    float m_mass = 1.0f;


    void UpdatePositionConstraints() const;
    void UpdateRotationConstraints() const;

    std::array<bool, 3> positionAxisLocks = { false, false, false };
    std::array<bool, 3> rotationAxisLocks = { false, false, false };
    int called = 0;
};

REGISTER_COMPONENT(Rigidbody);