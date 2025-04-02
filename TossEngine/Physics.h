#pragma once
#pragma warning(push)
#pragma warning(disable: 4244)
#include <reactphysics3d/reactphysics3d.h>
#pragma warning(pop)

#include "Utils.h"
#include <vector>

class Vector3;
class Rigidbody;
class Collider;

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
    Collider* collider = nullptr;
    Rigidbody* rigidbody = nullptr;
};

struct TOSSENGINE_API RaycastCallback : public reactphysics3d::RaycastCallback
{
    bool hit = false;
    Vector3 point;
    Vector3 normal;
    void* collider;
    void* rigidbody;

    virtual reactphysics3d::decimal notifyRaycastHit(const RaycastInfo& info) override;
};

struct TOSSENGINE_API RaycastDebugEntry {
    Vector3 origin;
    Vector3 endPoint;
    float lifetime = 1.0f;
};

class TOSSENGINE_API Physics : public rp3d::CollisionCallback
{
public:
    static Physics& GetInstance()
    {
        static Physics instance;
        return instance;
    }

    void Update(float deltaTime);

    void CleanUp();

    PhysicsWorld* GetWorld() const { return m_world; }

    PhysicsCommon& GetPhysicsCommon() { return m_commonSettings; }
    const PhysicsCommon& GetPhysicsCommon() const { return m_commonSettings; }
    PhysicsMaterialPtr GetDefaultPhysicsMaterial();

    void SetDebug(bool debug);
    bool GetDebug();
    void SetGravity(const Vector3& gravity);
    Vector3 GetGravity() const { return m_gravity; }


    RaycastHit Raycast(const Vector3& origin, const Vector3& direction, float maxDistance = 1000.0f);

    void DrawDebug(UniformData data);

    void onContact(const rp3d::CollisionCallback::CallbackData& data) override;
    void LoadWorld();
    void UnLoadWorld();

private:
    Physics() = default;
    ~Physics() = default;

    PhysicsCommon m_commonSettings;
    bool isDebug = false;
    PhysicsWorld* m_world = nullptr;
    PhysicsMaterialPtr m_defaultPhysicsMaterial = nullptr;
    Vector3 m_gravity = Vector3(0.0f, -9.81f, 0.0f);

    uint vertexCount = 0;
    ShaderPtr debugShader = nullptr;
    uint VBO_lines;
    uint VAO_lines;
    uint VBO_tris;
    uint VAO_tris; 
    vector<RaycastDebugEntry> m_raycastDebugEntries;

};