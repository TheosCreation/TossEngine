#include "Rigidbody.h"
#include "GameObject.h"
#include "Collider.h"
#include "PhysicsMaterial.h"

Rigidbody::~Rigidbody()
{
    if (m_Body) {
        Physics::GetInstance().GetWorld()->destroyRigidBody(m_Body);
        m_Body = nullptr;
        m_Collider = nullptr;
    }
}

json Rigidbody::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this)); // Store the component type
    data["bodyType"] = static_cast<int>(m_BodyType);
    data["mass"] = m_Body ? m_Body->getMass() : 0.0f;
    data["useGravity"] = m_UseGravity;
    data["positionConstraints"] = positionAxisLocks;
    data["rotationConstraints"] = rotationAxisLocks;

    return data;
}

void Rigidbody::deserialize(const json& data)
{
    if (data.contains("bodyType")) 
    {
        SetBodyType(static_cast<BodyType>(data["bodyType"].get<int>()));
    }
    if (data.contains("mass")) {
        SetMass(data["mass"].get<float>());
    }
    if (data.contains("useGravity")) {
        SetUseGravity(data["useGravity"].get<bool>());
    }
    if (data.contains("positionConstraints"))
        positionAxisLocks = data["positionConstraints"].get<std::array<bool, 3>>();

    if (data.contains("rotationConstraints"))
        rotationAxisLocks = data["rotationConstraints"].get<std::array<bool, 3>>();
}

void Rigidbody::OnInspectorGUI()
{
    ImGui::Text("Rigidbody Inspector - ID: %p", this);
    ImGui::Separator();


    // Body Type selector
    static const char* bodyTypes[] = { "Static", "Kinematic", "Dynamic" };
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
    bool useGravity = m_UseGravity;
    if (ImGui::Checkbox("Use Gravity", &useGravity))
    {
        SetUseGravity(useGravity);
        m_UseGravity = useGravity;
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

void Rigidbody::onCreate()
{
    PhysicsWorld* world = Physics::GetInstance().GetWorld();
    if (!world) return; // Early exit if world is null

    Transform transform = m_owner->m_transform;

    // Create rigid body at the GameObject�s position
    rp3d::Transform bodyTransform(
        rp3d::Vector3(transform.localPosition.x, transform.localPosition.y, transform.localPosition.z),
        rp3d::Quaternion(transform.localRotation.x, transform.localRotation.y, transform.localRotation.z, transform.localRotation.w)
    );

    // Create rigid body
    m_Body = world->createRigidBody(bodyTransform);
    m_Body->setUserData(this);
    //m_Body->setIsDebugEnabled(true);

    // Apply the default body type
    SetBodyType(m_BodyType);

    m_Collider = m_owner->getComponent<Collider>();
    if (m_Collider == nullptr)
    {
        m_owner->addComponent<Collider>();
    }
}

void Rigidbody::onStart()
{
    // Ensure m_Collider and m_Body are both valid
    if (m_Collider == nullptr) {
        Debug::LogError("Error: Collider is nullptr!");
        return;
    }
    if (m_Body == nullptr) {
        Debug::LogError("Error: Rigidbody Body is nullptr!");
        return;
    }

    // Retrieve the collider shape
    rp3d::CollisionShape* shape = m_Collider->GetColliderShape();

    // Set up collider material and other parameters
    bool isTrigger = m_Collider->GetTrigger();
    PhysicsMaterialPtr material = m_Collider->GetPhysicsMaterial();

    // Add the collider to the body
    rp3d::Collider* collider = m_Body->addCollider(shape, rp3d::Transform::identity());

    //collider->setIsTrigger(true);
    // Set the material properties
    rp3d::Material colliderMaterial = collider->getMaterial();
    colliderMaterial.setFrictionCoefficient(material->getDynamicFriction());
    colliderMaterial.setBounciness(material->getBounciness());
    
    // Apply the updated material to the collider
    collider->setMaterial(colliderMaterial);
    

    // Ensure the body is not sleeping when the simulation starts
    m_Body->setIsSleeping(false);
}


void Rigidbody::onUpdate(float deltaTime)
{
    if (!m_Body || m_BodyType == BodyType::STATIC) return;

    rp3d::Transform bodyTransform = m_Body->getTransform();
    rp3d::Vector3 position = bodyTransform.getPosition();
    rp3d::Quaternion rotation = bodyTransform.getOrientation();

    // Apply position constraints
    Vector3& pos = m_owner->m_transform.localPosition;
    if (!positionAxisLocks[0]) pos.x = position.x;
    if (!positionAxisLocks[1]) pos.y = position.y;
    if (!positionAxisLocks[2]) pos.z = position.z;

    // Apply rotation constraints
    Quaternion currentRot = m_owner->m_transform.localRotation;
    Quaternion newRot(rotation.w, rotation.x, rotation.y, rotation.z);

    // Simple approach: convert both to Euler angles
    Vector3 currentEuler = currentRot.ToEulerAngles();
    Vector3 newEuler = newRot.ToEulerAngles();

    if (!rotationAxisLocks[0]) currentEuler.x = newEuler.x;
    if (!rotationAxisLocks[1]) currentEuler.y = newEuler.y;
    if (!rotationAxisLocks[2]) currentEuler.z = newEuler.z;

    m_owner->m_transform.localRotation = Quaternion(currentEuler);
}

void Rigidbody::SetBodyType(BodyType type)
{
    if (m_Body) {
        m_BodyType = type;
        m_Body->setType(type);
    }
}

void Rigidbody::SetMass(float mass)
{
    if (m_Body) {
        m_Body->setMass(mass);
    }
}

void Rigidbody::SetUseGravity(bool useGravity)
{
    m_UseGravity = useGravity;
    if (m_Body) {
        m_Body->enableGravity(useGravity);
    }
}

rp3d::RigidBody* Rigidbody::GetBody()
{
    return m_Body;
}

Collider* Rigidbody::GetCollider()
{
    return m_Collider;
}

void Rigidbody::AddForce(const Vector3& force)
{
    if (m_Body) {
        m_Body->applyWorldForceAtCenterOfMass(rp3d::Vector3(force.x, force.y, force.z));
    }
}

void Rigidbody::AddTorque(const Vector3& torque)
{
    if (m_Body) {
        m_Body->applyWorldForceAtCenterOfMass(rp3d::Vector3(torque.x, torque.y, torque.z));
    }
}

Vector3 Rigidbody::GetLinearVelocity() const
{
    if (m_Body) {
        rp3d::Vector3 vel = m_Body->getLinearVelocity();
        return Vector3(vel.x, vel.y, vel.z);
    }
    return Vector3();
}

void Rigidbody::SetLinearVelocity(const Vector3& velocity)
{
    if (m_Body) {
        m_Body->setLinearVelocity(rp3d::Vector3(velocity.x, velocity.y, velocity.z));
    }
}

Vector3 Rigidbody::GetAngularVelocity() const
{
    if (m_Body) {
        rp3d::Vector3 angVel = m_Body->getAngularVelocity();
        return Vector3(angVel.x, angVel.y, angVel.z);
    }
    return Vector3();
}

void Rigidbody::SetAngularVelocity(const Vector3& velocity)
{
    if (m_Body) {
        m_Body->setAngularVelocity(rp3d::Vector3(velocity.x, velocity.y, velocity.z));
    }
}

void Rigidbody::SetPositionConstraints(bool lockX, bool lockY, bool lockZ)
{
    positionAxisLocks = { lockX, lockY, lockZ };
    if (m_Body) {
        m_Body->setLinearLockAxisFactor(rp3d::Vector3(lockX ? 0.0f : 1.0f, lockY ? 0.0f : 1.0f, lockZ ? 0.0f : 1.0f));
    }
}

void Rigidbody::SetRotationConstraints(bool lockX, bool lockY, bool lockZ)
{
    rotationAxisLocks = { lockX, lockY, lockZ };
    if (m_Body) {
        m_Body->setAngularLockAxisFactor(rp3d::Vector3(lockX ? 0.0f : 1.0f, lockY ? 0.0f : 1.0f, lockZ ? 0.0f : 1.0f));
    }
}

void Rigidbody::OnCollisionEnter(Rigidbody* collidedRb)
{
    Collider* collider = collidedRb->GetCollider();
    m_owner->CallOnCollisionEnterCallbacks(collider);
}

void Rigidbody::OnCollisionExit(Rigidbody* collidedRb)
{
    Collider* collider = collidedRb->GetCollider();
    m_owner->CallOnCollisionExitCallbacks(collider);
}
