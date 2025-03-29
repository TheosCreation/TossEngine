#include "Physics.h"
#include "Vector3.h"
#include "ResourceManager.h"
#include "PhysicsMaterial.h"
#include "Rigidbody.h"

reactphysics3d::decimal RaycastCallback::notifyRaycastHit(const RaycastInfo& info)
{
    hit = true;
    point = info.worldPoint;
    normal = info.worldNormal;
    return info.hitFraction;
}

void Physics::Init()
{
    if (m_world)
    {
        m_commonSettings.destroyPhysicsWorld(m_world);
        m_world = nullptr;
    }

    PhysicsMaterialDesc physicsMaterialDesc; //full defaults set in the code atm
    m_defaultPhysicsMaterial = std::make_shared<PhysicsMaterial>(physicsMaterialDesc, "DefaultPhysicsMaterial", nullptr);
    
    rp3d::PhysicsWorld::WorldSettings settings;
    settings.gravity = static_cast<rp3d::Vector3>(m_gravity);
    settings.defaultPositionSolverNbIterations = 20;  // Default is usually 6
    settings.defaultVelocitySolverNbIterations = 10;  // Default is usually 4
    m_world = m_commonSettings.createPhysicsWorld(settings);
}


void Physics::Update(float deltaTime)
{
    if (m_world)
    {
        m_world->update(deltaTime);
        m_world->testCollision(*this);
    }
}

void Physics::CleanUp()
{
    if (m_world)
    {
        m_commonSettings.destroyPhysicsWorld(m_world);
        m_world = nullptr;
    }
}

PhysicsMaterialPtr Physics::GetDefaultPhysicsMaterial()
{
    return m_defaultPhysicsMaterial;
}

void Physics::SetGravity(const Vector3& gravity)
{
    m_gravity = gravity;

    if (m_world)
    {
        m_world->setGravity(static_cast<rp3d::Vector3>(m_gravity));
    }
}

RaycastHit Physics::Raycast(const Vector3& origin, const Vector3& direction, float maxDistance)
{
    Ray ray(static_cast<rp3d::Vector3>(origin), static_cast<rp3d::Vector3>(direction));

    RaycastHit hitResult;
    RaycastCallback callback;

    m_world->raycast(ray, &callback);

    // Fill out result manually
    hitResult.hasHit = callback.hit;
    hitResult.point = Vector3(callback.point);
    hitResult.normal = Vector3(callback.normal);
    hitResult.distance = callback.hit ? Vector3::Distance(origin, hitResult.point) : maxDistance;

    return hitResult;
}

void Physics::onContact(const rp3d::CollisionCallback::CallbackData& data)
{// Get the number of contact pairs
    int nbContactPairs = data.getNbContactPairs();

    // Log if no contact pairs are found
    if (nbContactPairs == 0) {
        std::cerr << "No contact pairs detected." << std::endl;
    }

    // Iterate over all the contact pairs
    for (int i = 0; i < nbContactPairs; i++) {
        const rp3d::CollisionCallback::ContactPair& contactPair = data.getContactPair(i);

        // Get the number of contact points for the contact pair
        int nbContactPoints = contactPair.getNbContactPoints();

        // If there are no contact points, log the failure
        if (nbContactPoints == 0) {
            std::cerr << "Contact pair " << i << " has no contact points!" << std::endl;
        }

        // Proceed only if there are contact points
        if (nbContactPoints > 0) {
            // Get the colliders involved in the collision
            rp3d::Collider* collider1 = contactPair.getCollider1();
            rp3d::Collider* collider2 = contactPair.getCollider2();

            // Get the Rigidbody associated with the colliders
            rp3d::Body* body1 = collider1->getBody();
            rp3d::Body* body2 = collider2->getBody();

            // Check if the bodies are valid
            if (body1 && body2) {
                // Get the custom Rigidbody components from the bodies
                Rigidbody* customRigidbody1 = static_cast<Rigidbody*>(body1->getUserData());
                Rigidbody* customRigidbody2 = static_cast<Rigidbody*>(body2->getUserData());

                // If a custom Rigidbody exists for the first body, call OnCollisionEnter
                if (customRigidbody1) {
                    customRigidbody1->OnCollisionEnter(customRigidbody2); // Pass the collision object if needed
                }

                // If a custom Rigidbody exists for the second body, call OnCollisionEnter
                if (customRigidbody2) {
                    customRigidbody2->OnCollisionEnter(customRigidbody1); // Pass the collision object if needed
                }
            }
            else {
                std::cerr << "Invalid body detected in contact pair." << std::endl;
            }
        }
    }
}