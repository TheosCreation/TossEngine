#pragma once
#pragma warning(push)
#pragma warning(disable: 4244)
#include <reactphysics3d/reactphysics3d.h>
#pragma warning(pop)

#include "Utils.h"
#include <vector>

class Vector3;

using PhysicsWorld = reactphysics3d::PhysicsWorld;
using Ray = reactphysics3d::Ray;
using PhysicsCommon = reactphysics3d::PhysicsCommon;
using RaycastInfo = reactphysics3d::RaycastInfo;

struct TOSSENGINE_API RaycastHit
{
    bool hasHit = false;
    Vector3 point;
    Vector3 normal;
    float distance = 0.0f;
};

struct TOSSENGINE_API RaycastCallback : public reactphysics3d::RaycastCallback
{
    bool hit = false;
    Vector3 point;
    Vector3 normal;

    virtual reactphysics3d::decimal notifyRaycastHit(const RaycastInfo& info) override;
};

class TOSSENGINE_API Physics
{
public:
    static Physics& GetInstance()
    {
        static Physics instance;
        return instance;
    }

    void Init();

    void Update(float deltaTime);

    void CleanUp();

    PhysicsWorld* GetWorld() const { return m_world; }

    PhysicsCommon& GetPhysicsCommon() { return m_commonSettings; }
    const PhysicsCommon& GetPhysicsCommon() const { return m_commonSettings; }

    void SetGravity(const Vector3& gravity);
    Vector3 GetGravity() const { return m_gravity; }


    RaycastHit Raycast(const Vector3& origin, const Vector3& direction, float maxDistance = 1000.0f);

    void DrawDebug()
    {
        // Placeholder: You can extend this with ImGui drawing or render lines manually with your debug renderer
    }

private:
    Physics() = default;
    ~Physics() = default;

    PhysicsCommon m_commonSettings;
    PhysicsWorld* m_world = nullptr;

    Vector3 m_gravity = Vector3(0.0f, -9.81f, 0.0f); //can be adjusted at runtime in editor or ingame
};