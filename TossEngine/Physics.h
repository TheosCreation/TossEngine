#pragma once
#pragma warning(push)
#pragma warning(disable: 4244)
#include <reactphysics3d/reactphysics3d.h>
#pragma warning(pop)

#include "Utils.h"
#include "LayerManager.h"
#include <vector>
#include <unordered_set>

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

struct TriggerPair {
    rp3d::Collider* collider1;
    rp3d::Collider* collider2;

    bool operator==(const TriggerPair& other) const {
        return (collider1 == other.collider1 && collider2 == other.collider2) ||
            (collider1 == other.collider2 && collider2 == other.collider1);
    }
};

// Custom hash function for TriggerPair.
namespace std {
    template <>
    struct hash<TriggerPair> {
        std::size_t operator()(const TriggerPair& pair) const {
            auto h1 = std::hash<rp3d::Collider*>()(pair.collider1);
            auto h2 = std::hash<rp3d::Collider*>()(pair.collider2);
            return h1 ^ h2;
        }
    };
}
// A structure representing a collision pair (order independent)
struct CollisionPair {
    rp3d::Collider* collider1;
    rp3d::Collider* collider2;

    // Define equality as order independent.
    bool operator==(const CollisionPair& other) const {
        return (collider1 == other.collider1 && collider2 == other.collider2) ||
            (collider1 == other.collider2 && collider2 == other.collider1);
    }
};

// Custom hash function for CollisionPair.
namespace std {
    template <>
    struct hash<CollisionPair> {
        std::size_t operator()(const CollisionPair& pair) const {
            // Combine the hash of both pointers. The XOR approach is simple but effective.
            auto h1 = std::hash<rp3d::Collider*>()(pair.collider1);
            auto h2 = std::hash<rp3d::Collider*>()(pair.collider2);
            return h1 ^ h2;
        }
    };
}
class TOSSENGINE_API Physics : public rp3d::EventListener
{
public:
    static Physics& GetInstance()
    {
        static Physics instance;
        return instance;
    }

    void Update();

    void CleanUp();

    PhysicsWorld* GetWorld() const { return m_world; }
    PhysicsWorld* GetPrefabWorld() const { return m_prefabWorld; }

    PhysicsCommon& GetPhysicsCommon() { return m_commonSettings; }
    const PhysicsCommon& GetPhysicsCommon() const { return m_commonSettings; }
    PhysicsMaterialPtr GetDefaultPhysicsMaterial();

    void SetDebug(bool debug);
    bool GetDebug();
    void SetGravity(const Vector3& gravity);
    Vector3 GetGravity() const { return m_gravity; }


    RaycastHit Raycast(const Vector3& origin, const Vector3& direction, float maxDistance = 1000.0f, LayerBit hitLayers = 0xFFFFFFFF);

    void DrawDebug(UniformData data);

    void onContact(const rp3d::CollisionCallback::CallbackData& data) override;
    void onTrigger(const rp3d::OverlapCallback::CallbackData& data) override;
    void LoadWorld();
    void UnLoadWorld();
    void LoadPrefabWorld();
    void UnLoadPrefabWorld();

private:
    Physics() = default;
    ~Physics() = default;

    PhysicsCommon m_commonSettings;
    bool isDebug = false;
    PhysicsWorld* m_world = nullptr;
    PhysicsWorld* m_prefabWorld = nullptr;
    PhysicsMaterialPtr m_defaultPhysicsMaterial = nullptr;
    Vector3 m_gravity = Vector3(0.0f, -9.81f, 0.0f);

    std::unordered_set<TriggerPair> m_previousTriggerPairs;
    std::unordered_set<CollisionPair> m_previousCollisionPairs;

    uint vertexCount = 0;
    uint VBO_lines;
    uint VAO_lines;
    uint VBO_tris;
    uint VAO_tris; 
    vector<RaycastDebugEntry> m_raycastDebugEntries;

};