#include "Rigidbody.h"
#include "GameObject.h"
#include "GameObjectManager.h"
#include "Scene.h"

Rigidbody::~Rigidbody()
{
	if (m_Body) {
		m_Body->removeCollider(m_Collider); // Ensure collider is removed
        Physics::GetInstance().GetWorld()->destroyRigidBody(m_Body);
		m_Body = nullptr;
		m_Collider = nullptr;
	}
}

json Rigidbody::serialize() const
{
	json colliderJson;
    // Serialize using stored values
    if (m_Collider) {
        if (auto boxShape = dynamic_cast<rp3d::BoxShape*>(m_Collider->getCollisionShape())) {
            colliderJson = {
                {"type", "box"},
                {"size", {m_boxColliderSize.x, m_boxColliderSize.y, m_boxColliderSize.z}}
            };
        }
        else if (auto sphereShape = dynamic_cast<rp3d::SphereShape*>(m_Collider->getCollisionShape())) {
            colliderJson = {
                {"type", "sphere"},
                {"radius", m_radius}
            };
        }
        else if (auto capsuleShape = dynamic_cast<rp3d::CapsuleShape*>(m_Collider->getCollisionShape()))
        {
            colliderJson = {
                {"type", "capsule"},
                {"radius", m_radius},
                {"height", m_height},
            };
        }
    }

	json data;
	data["type"] = getClassName(typeid(*this));
	data["bodyType"] = static_cast<int>(m_BodyType);
	data["mass"] = m_Body ? m_Body->getMass() : 0.0f;
	data["useGravity"] = m_Body ? m_Body->isGravityEnabled() : false;
	data["collider"] = colliderJson; 
    data["positionConstraints"] = positionAxisLocks;
    data["rotationConstraints"] = rotationAxisLocks;

	return data;
}

void Rigidbody::deserialize(const json& data)
{
	if (data.contains("bodyType")) {

		SetBodyType(static_cast<BodyType>(data["bodyType"].get<int>()));
	}
	if (data.contains("mass")) {
		SetMass(data["mass"].get<float>());
	}
	if (data.contains("useGravity")) {
		SetUseGravity(data["useGravity"].get<bool>());
	}
	if (data.contains("collider")) {
		auto colliderData = data["collider"];
		if (colliderData.contains("type")) {
			std::string type = colliderData["type"].get<std::string>();

			if (type == "box" && colliderData.contains("size")) {
				auto size = colliderData["size"];
				SetBoxCollider(Vector3(size[0], size[1], size[2]));
			}
			else if (type == "sphere" && colliderData.contains("radius")) {
				SetSphereCollider(colliderData["radius"].get<float>());
			}
            else if (type == "capsule" && colliderData.contains("radius") && colliderData.contains("height")) {
				SetCapsuleCollider(colliderData["radius"].get<float>(), colliderData["height"].get<float>());
			}
		}
	}
    
    if (data.contains("positionConstraints"))
        positionAxisLocks = data["positionConstraints"].get<std::array<bool, 3>>();

    if (data.contains("rotationConstraints"))
        rotationAxisLocks = data["rotationConstraints"].get<std::array<bool, 3>>();

	m_Body->setIsSleeping(false);
}

void Rigidbody::onCreate()
{
	PhysicsWorld* world = Physics::GetInstance().GetWorld();
	if (!world) return; // Early exit if world is null

	Transform transform = m_owner->m_transform;

	// Create rigid body at the GameObject’s position
	rp3d::Transform bodyTransform(
		rp3d::Vector3(transform.position.x, transform.position.y, transform.position.z),
		rp3d::Quaternion(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w)
	);

	// Create rigid body
	m_Body = world->createRigidBody(bodyTransform);
	//m_Body->setIsDebugEnabled(true);

	// Apply the default body type
	SetBodyType(m_BodyType);
}

void Rigidbody::onStart()
{
    if (m_Body) {
        m_Body->setLinearLockAxisFactor(rp3d::Vector3(
            positionAxisLocks[0] ? 0.0f : 1.0f,
            positionAxisLocks[1] ? 0.0f : 1.0f,
            positionAxisLocks[2] ? 0.0f : 1.0f
        ));

        m_Body->setAngularLockAxisFactor(rp3d::Vector3(
            rotationAxisLocks[0] ? 0.0f : 1.0f,
            rotationAxisLocks[1] ? 0.0f : 1.0f,
            rotationAxisLocks[2] ? 0.0f : 1.0f
        ));
    }
}

void Rigidbody::onUpdate(float deltaTime)
{
    if (!m_Body) return;

    rp3d::Transform bodyTransform = m_Body->getTransform();
    rp3d::Vector3 position = bodyTransform.getPosition();
    rp3d::Quaternion rotation = bodyTransform.getOrientation();

    // Apply position constraints
    Vector3& pos = m_owner->m_transform.position;
    if (!positionAxisLocks[0]) pos.x = position.x;
    if (!positionAxisLocks[1]) pos.y = position.y;
    if (!positionAxisLocks[2]) pos.z = position.z;

    // Apply rotation constraints
    Quaternion currentRot = m_owner->m_transform.rotation;
    Quaternion newRot(rotation.w, rotation.x, rotation.y, rotation.z);

    // Simple approach: convert both to Euler angles
    Vector3 currentEuler = currentRot.ToEulerAngles();
    Vector3 newEuler = newRot.ToEulerAngles();

    if (!rotationAxisLocks[0]) currentEuler.x = newEuler.x;
    if (!rotationAxisLocks[1]) currentEuler.y = newEuler.y;
    if (!rotationAxisLocks[2]) currentEuler.z = newEuler.z;

    m_owner->m_transform.rotation = Quaternion(currentEuler);
}

void Rigidbody::SetBodyType(BodyType type)
{
	if (!m_Body) return;

	m_BodyType = type;

	switch (type) {
	case BodyType::Static:
		m_Body->setType(rp3d::BodyType::STATIC);
		break;
	case BodyType::Kinematic:
		m_Body->setType(rp3d::BodyType::KINEMATIC);
		break;
	case BodyType::Dynamic:
		m_Body->setType(rp3d::BodyType::DYNAMIC);
		break;
	}
}

void Rigidbody::SetMass(float mass)
{
	if (!m_Body) return;

	m_Body->setMass(mass);
}

void Rigidbody::SetUseGravity(bool useGravity)
{
	if (!m_Body) return;

	m_Body->enableGravity(useGravity);
}

void Rigidbody::SetBoxCollider(const Vector3& size) {
    if (!m_Body) return;
    m_boxColliderSize = size;

    PhysicsCommon& physicsCommon = Physics::GetInstance().GetPhysicsCommon();

    if (m_Collider) {
        m_Body->removeCollider(m_Collider);
        m_Collider = nullptr;
    }

    rp3d::Vector3 boxSize = { size.x * 0.5f, size.y * 0.5f, size.z * 0.5f }; // half extents
    rp3d::BoxShape* boxShape = physicsCommon.createBoxShape(boxSize);

    m_Collider = m_Body->addCollider(boxShape, rp3d::Transform::identity());
}

void Rigidbody::SetSphereCollider(float radius) {
    if (!m_Body) return;
    m_radius = radius;

    PhysicsCommon& physicsCommon = Physics::GetInstance().GetPhysicsCommon();

    if (m_Collider) {
        m_Body->removeCollider(m_Collider);
        m_Collider = nullptr;
    }

    rp3d::SphereShape* sphereShape = physicsCommon.createSphereShape(radius);
    m_Collider = m_Body->addCollider(sphereShape, rp3d::Transform::identity());
}

void Rigidbody::SetCapsuleCollider(float radius, float height)
{
    if (!m_Body) return;
    m_radius = radius; 
    m_height = height;

    PhysicsCommon& physicsCommon = Physics::GetInstance().GetPhysicsCommon();

    if (m_Collider) {
        m_Body->removeCollider(m_Collider);
        m_Collider = nullptr;
    }

    rp3d::CapsuleShape* capsuleShape = physicsCommon.createCapsuleShape(radius, height);
    m_Collider = m_Body->addCollider(capsuleShape, rp3d::Transform::identity());
}

void Rigidbody::RemoveCollider()
{
    if (m_Collider) {
        m_Body->removeCollider(m_Collider);
        m_Collider = nullptr;
    }
}