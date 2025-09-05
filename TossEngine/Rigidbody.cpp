#include "Rigidbody.h"
#include "GameObject.h"
#include "Collider.h"
#include "PhysicsMaterial.h"

json Rigidbody::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this)); // Store the component type
    data["bodyType"] = static_cast<int>(m_BodyType);
    data["mass"] = m_mass;
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
    if (ImGui::DragFloat("Mass", &m_mass, 0.1f, 0.1f, 10.0f))
    {
        SetMass(m_mass);
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
    SetIsDebugEnabled(true);
}

void Rigidbody::OnGameObjectDeSelected()
{
    SetIsDebugEnabled(false);
}

void Rigidbody::onCreate()
{
    PhysicsWorld* world = m_owner->getWorld();

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
    // Ensure m_collider and m_Body are both valid
    if (m_Collider == nullptr) {
        Debug::LogError("Error: Collider is nullptr!");
        return;
    }
    if (m_Body == nullptr) {
        Debug::LogError("Error: Rigidbody Body is nullptr!");
    }

    UpdatePositionConstraints();
    UpdateRotationConstraints();

    // Ensure the body is not sleeping when the simulation starts
    if (m_Body)
    {
        UpdateBodyTransform();
        m_Body->setIsSleeping(false);
    }
}

void Rigidbody::onUpdate()
{
    if (!m_Body || !m_owner) return;

    Mat4 worldMatrix = m_owner->m_transform.GetMatrix();

    // Extract position and rotation from the world matrix
    Vector3 physPos = Vector3::ExtractTranslation(worldMatrix);
    Quaternion physRot = Quaternion::ExtractRotation(worldMatrix);

    // Ensure quaternion is normalized
    if (physRot.Magnitude() > 0.00001f)
        physRot.Normalize();
    else
        physRot = Quaternion::Identity();

    // --- 2) Apply physics to transform (dynamic bodies) ---
    if (m_BodyType != BodyType::KINEMATIC)
    {
        // Get physics transform
        rp3d::Transform bodyTransform = m_Body->getTransform();
        Vector3 bodyPos(bodyTransform.getPosition().x,
            bodyTransform.getPosition().y,
            bodyTransform.getPosition().z);
        Quaternion bodyRot(bodyTransform.getOrientation().x,
            bodyTransform.getOrientation().y,
            bodyTransform.getOrientation().z,
            bodyTransform.getOrientation().w);

        if (bodyRot.Magnitude() > rp3d::MACHINE_EPSILON)
            bodyRot.Normalize();
        else
            bodyRot = Quaternion::Identity();

        // --- 2a) Apply position constraints ---
        Vector3 newPos = bodyPos;
        if (positionAxisLocks[0]) newPos.x = physPos.x;
        if (positionAxisLocks[1]) newPos.y = physPos.y;
        if (positionAxisLocks[2]) newPos.z = physPos.z;

        // --- 2b) Apply rotation constraints ---
        Quaternion newRot = bodyRot;
        if (rotationAxisLocks[0] || rotationAxisLocks[1] || rotationAxisLocks[2])
        {
            Vector3 bodyEuler = bodyRot.ToEulerAngles();
            Vector3 physEuler = physRot.ToEulerAngles();

            if (rotationAxisLocks[0]) bodyEuler.x = physEuler.x;
            if (rotationAxisLocks[1]) bodyEuler.y = physEuler.y;
            if (rotationAxisLocks[2]) bodyEuler.z = physEuler.z;

            newRot = Quaternion(bodyEuler);
        }

        // --- 2c) Write back to Transform ---
        if (m_owner->m_transform.parent)
        {
            Mat4 parentInv = m_owner->m_transform.parent->GetMatrix().Inverse();
            m_owner->m_transform.localPosition = parentInv * newPos;
            Quaternion parentRotInv = m_owner->m_transform.parent->rotation.Inverse();
            m_owner->m_transform.localRotation = parentRotInv * newRot;
        }
        else
        {
            m_owner->m_transform.position = newPos;
            m_owner->m_transform.rotation = newRot;
        }
    }
    else // Kinematic bodies
    {
        // For kinematic, overwrite the physics transform with the visual transform
        rp3d::Transform updatedBody(
            rp3d::Vector3(physPos.x, physPos.y, physPos.z),
            rp3d::Quaternion(physRot.x, physRot.y, physRot.z, physRot.w)
        );
        m_Body->setTransform(updatedBody);
    }
}

void Rigidbody::onUpdateInternal()
{
    if (Time::TimeScale == 0.0f)
    {
        UpdateBodyTransform();
    }
}

void Rigidbody::UpdateBodyTransform()
{
    if (m_Body)
    {
        Vector3 worldTranslation = m_owner->m_transform.position;
        if (m_owner->m_transform.parent)
        {
            worldTranslation = m_owner->m_transform.localPosition;
        }
        Quaternion worldRotation = m_owner->m_transform.rotation;
        if (m_owner->m_transform.parent)
        {
            worldRotation = m_owner->m_transform.localRotation;
        }

        rp3d::Transform bodyTransform(
            rp3d::Vector3(worldTranslation.x, worldTranslation.y, worldTranslation.z),
            rp3d::Quaternion(worldRotation.x, worldRotation.y, worldRotation.z, worldRotation.w)
        );

        m_Body->setTransform(bodyTransform);
    }
}

void Rigidbody::onDestroy()
{
    if (!m_Body)
    {
        Debug::LogError("Tried to destroy null Rigidbody");
        return;
    }

    m_Body->setIsActive(false);

    if (m_owner && m_owner->getWorld())
    {
        m_owner->getWorld()->destroyRigidBody(m_Body);
    }
    else
    {
        Debug::LogError("Trying to destroy a Rigidbody not in the world");
    }

    m_Body = nullptr;
    m_Collider = nullptr;
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

void Rigidbody::SetIsDebugEnabled(bool enabled) const
{
    if (m_Body && Physics::GetInstance().GetDebug())
    {
        m_Body->setIsDebugEnabled(enabled);
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
    if (m_Body && force.Length() > 0.1f * m_Body->getMass()) {
        m_Body->applyWorldForceAtCenterOfMass(rp3d::Vector3(force.x, force.y, force.z));
    }
}

void Rigidbody::AddTorque(const Vector3& torque) const
{
    if (m_Body && torque.Length() > 0.1f * m_Body->getMass())
    {
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
