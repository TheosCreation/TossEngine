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
    }

	json data;
	data["type"] = getClassName(typeid(*this));
	data["bodyType"] = static_cast<int>(m_BodyType);
	data["mass"] = m_Body ? m_Body->getMass() : 0.0f;
	data["useGravity"] = m_Body ? m_Body->isGravityEnabled() : false;
	data["collider"] = colliderJson;

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
		}
	}

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
    if (!m_Body) return;

    Vector3 worldScale = m_owner->m_transform.GetWorldScale();

    // Re-apply the collider with correct world scale
    if (m_Collider)
    {
        auto shape = m_Collider->getCollisionShape();
        if (dynamic_cast<rp3d::BoxShape*>(shape))
        {
            Vector3 scaledSize = m_boxColliderSize * worldScale;
            SetBoxCollider(m_boxColliderSize); // SetBoxCollider already scales internally
        }
        else if (dynamic_cast<rp3d::SphereShape*>(shape))
        {
            float maxAxis = std::max({ worldScale.x, worldScale.y, worldScale.z });
            float scaledRadius = m_radius * maxAxis;
            SetSphereCollider(m_radius); // Same here — let it scale inside the setter
        }
    }
}

void Rigidbody::onFixedUpdate(float fixedDeltaTime)
{
	if (!m_Body) return;

	rp3d::Transform bodyTransform = m_Body->getTransform();
	rp3d::Vector3 position = bodyTransform.getPosition();
	rp3d::Quaternion rotation = bodyTransform.getOrientation();

	m_owner->m_transform.position = { position.x, position.y, position.z };
	m_owner->m_transform.rotation = { rotation.x, rotation.y, rotation.z, rotation.w };
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

void Rigidbody::RemoveCollider()
{
    if (m_Collider) {
        m_Body->removeCollider(m_Collider);
        m_Collider = nullptr;
    }
}