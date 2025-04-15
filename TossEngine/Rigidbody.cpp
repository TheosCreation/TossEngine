#include "Rigidbody.h"
#include "GameObject.h"
#include "Collider.h"
#include "PhysicsMaterial.h"

Rigidbody::~Rigidbody()
{
    Rigidbody::onDestroy(); //last resort to call destroy for no errors
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
    if (ImGui::DragFloat("Mass", &mass, 0.1f, 0.1f, 10.0f))
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

void Rigidbody::OnGameObjectSelected()
{
    if (m_Body && Physics::GetInstance().GetDebug())
    {
        m_Body->setIsDebugEnabled(true);
    }
}

void Rigidbody::OnGameObjectDeSelected()
{
    if (m_Body && Physics::GetInstance().GetDebug())
    {
        m_Body->setIsDebugEnabled(false);
    }
}

void Rigidbody::onCreate()
{
    PhysicsWorld* world = m_owner->getWorld();
    if (!world)
    {
        Debug::LogError("Rigidbody not able to create body in world", true);
    }

    Transform transform = m_owner->m_transform;

    // Create rigid body at the GameObject's position
    rp3d::Transform bodyTransform;

    // Create rigid body
    m_Body = world->createRigidBody(bodyTransform);
    m_Body->setUserData(this);

    // Apply the default body type
    SetBodyType(m_BodyType);
}

void Rigidbody::onCreateLate()
{
    UpdateBodyTransform();

    m_Collider = m_owner->getComponent<Collider>();
    if (m_Collider == nullptr)
    {
        m_Collider = m_owner->addComponent<Collider>();
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

    UpdatePositionConstraints();
    UpdateRotationConstraints();

    // Ensure the body is not sleeping when the simulation starts
    m_Body->setIsSleeping(false);
}


void Rigidbody::onUpdate()
{
    if (!m_Body || m_BodyType == BodyType::STATIC || (m_BodyType == BodyType::KINEMATIC && m_Collider->GetTrigger()))
    {
        UpdateBodyTransform();
    }
    

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

void Rigidbody::onUpdateInternal()
{
    if (Time::TimeScale == 0.0f)
    {
        UpdateBodyTransform();
    }
}

void Rigidbody::UpdateBodyTransform() const
{
    if (m_Body)
    {
        Mat4 worldMatrix = m_owner->m_transform.GetMatrix();

        Vector3 worldTranslation = Vector3::ExtractTranslation(worldMatrix);
        Quaternion worldRotation = Quaternion::ExtractRotation(worldMatrix);

        rp3d::Transform bodyTransform(
            rp3d::Vector3(worldTranslation.x, worldTranslation.y, worldTranslation.z),
            rp3d::Quaternion(worldRotation.x, worldRotation.y, worldRotation.z, worldRotation.w)
        );
        m_Body->setTransform(bodyTransform);
    }
}

void Rigidbody::onDestroy()
{
    if (m_Body) {
        PhysicsWorld* world = m_owner->getWorld();
        if (world)
        {
            world->destroyRigidBody(m_Body);
        }
        m_Body = nullptr;
        m_Collider = nullptr;
    }
}

void Rigidbody::SetBodyType(BodyType type)
{
    m_BodyType = type;
    if (m_Body) {
        m_Body->setType(type);
    }
}

void Rigidbody::SetMass(float mass) const
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

rp3d::RigidBody* Rigidbody::GetBody() const
{
    return m_Body;
}

Collider* Rigidbody::GetCollider() const
{
    return m_Collider;
}

void Rigidbody::AddForce(const Vector3& force) const
{
    if (force.Length() > 0.1f)
    {
        if (m_Body) {
            m_Body->applyWorldForceAtCenterOfMass(rp3d::Vector3(force.x, force.y, force.z));
        }
    }
}

void Rigidbody::AddTorque(const Vector3& torque) const
{
    if (torque.Length() > 0.1f)
    {
        if (m_Body) {
            m_Body->applyWorldForceAtCenterOfMass(rp3d::Vector3(torque.x, torque.y, torque.z));
        }
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

void Rigidbody::SetLinearVelocity(const Vector3& velocity) const
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
    return Vector3(0.0f);
}

void Rigidbody::SetAngularVelocity(const Vector3& velocity) const
{
    if (m_Body) {
        m_Body->setAngularVelocity(rp3d::Vector3(velocity.x, velocity.y, velocity.z));
    }
}

void Rigidbody::SetPositionConstraints(bool lockX, bool lockY, bool lockZ)
{
    positionAxisLocks = { lockX, lockY, lockZ };
    UpdatePositionConstraints();
}

void Rigidbody::SetRotationConstraints(bool lockX, bool lockY, bool lockZ)
{
    rotationAxisLocks = { lockX, lockY, lockZ };
    UpdateRotationConstraints();
}

void Rigidbody::OnCollisionEnter(Rigidbody* collidedRb) const
{
    Collider* collider = collidedRb->GetCollider();
    m_owner->CallOnCollisionEnterCallbacks(collider);
}

void Rigidbody::OnCollisionExit(Rigidbody* collidedRb) const
{
    Collider* collider = collidedRb->GetCollider();
    m_owner->CallOnCollisionExitCallbacks(collider);
}

void Rigidbody::UpdatePositionConstraints() const
{
    if (m_Body) {
        m_Body->setLinearLockAxisFactor(rp3d::Vector3(positionAxisLocks[0] ? 0.0f : 1.0f, positionAxisLocks[1] ? 0.0f : 1.0f, positionAxisLocks[2] ? 0.0f : 1.0f));
    }
}

void Rigidbody::UpdateRotationConstraints() const
{
    if (m_Body) {
        m_Body->setAngularLockAxisFactor(rp3d::Vector3(rotationAxisLocks[0] ? 0.0f : 1.0f, rotationAxisLocks[1] ? 0.0f : 1.0f, rotationAxisLocks[2] ? 0.0f : 1.0f));
    }
}
