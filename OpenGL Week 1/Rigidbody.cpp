#include "Rigidbody.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "Scene.h"

Rigidbody::Rigidbody()
{
}

Rigidbody::~Rigidbody()
{
	if (m_Body) {
		m_owner->getGameObjectManager()->getScene()->GetPhysicsWorld()->destroyRigidBody(m_Body);
	}
}

void Rigidbody::onCreate()
{
	Scene* scene = m_owner->getGameObjectManager()->getScene();


	// Get the physics world from the scene
	rp3d::PhysicsWorld* world = scene->GetPhysicsWorld();

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

	// Get physics common object from the Scene
	Scene* scene = m_owner->getGameObjectManager()->getScene();
	rp3d::PhysicsCommon& physicsCommon = scene->GetPhysicsCommon();

	// Create a box shape
	rp3d::Vector3 boxSize = { size.x, size.y, size.z };
	rp3d::BoxShape* boxShape = physicsCommon.createBoxShape(boxSize * 0.5f);

	// Attach collider to rigidbody
	m_Collider = m_Body->addCollider(boxShape, rp3d::Transform::identity());
}

void Rigidbody::SetSphereCollider(float radius) {
	if (!m_Body) return;

	Scene* scene = m_owner->getGameObjectManager()->getScene();
	rp3d::PhysicsCommon& physicsCommon = scene->GetPhysicsCommon();

	// Create a sphere shape
	rp3d::SphereShape* sphereShape = physicsCommon.createSphereShape(radius);

	// Attach collider
	m_Collider = m_Body->addCollider(sphereShape, rp3d::Transform::identity());
}