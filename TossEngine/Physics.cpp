#include "Physics.h"
#include "Vector3.h"

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

    rp3d::PhysicsWorld::WorldSettings settings;
    settings.gravity = static_cast<rp3d::Vector3>(m_gravity);
    m_world = m_commonSettings.createPhysicsWorld(settings);
}


void Physics::Update(float deltaTime)
{
    if (m_world)
        m_world->update(deltaTime);
}

void Physics::CleanUp()
{
    if (m_world)
    {
        m_commonSettings.destroyPhysicsWorld(m_world);
        m_world = nullptr;
    }
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