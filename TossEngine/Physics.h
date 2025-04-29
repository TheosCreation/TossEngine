/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : Physics.h
Description : Manages the physics simulation using ReactPhysics3D, including collision detection, raycasting, and debug visualization.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#pragma warning(push)
#pragma warning(disable: 4244) // Disable possible loss of data warning
#include <reactphysics3d/reactphysics3d.h>
#pragma warning(pop)

#include "Utils.h"
#include "LayerManager.h"
#include <vector>
#include <unordered_set>

// Forward Declarations
class Vector3;
class Rigidbody;
class Collider;

// Aliases for ReactPhysics3D types
using PhysicsWorld = reactphysics3d::PhysicsWorld;
using Ray = reactphysics3d::Ray;
using PhysicsCommon = reactphysics3d::PhysicsCommon;
using RaycastInfo = reactphysics3d::RaycastInfo;

/**
 * @struct Collision
 * @brief Represents a collision contact between two colliders.
 */
struct TOSSENGINE_API Collision
{
    Vector3 contactPoint;
    Collider* thisCollider;
    Collider* otherCollider;
};

/**
 * @struct RaycastHit
 * @brief Result of a raycast query.
 */
struct TOSSENGINE_API RaycastHit
{
    bool hasHit = false;
    Vector3 point;
    Vector3 normal;
    float distance = 0.0f;
    Collider* collider = nullptr;
    Rigidbody* rigidbody = nullptr;
};


/**
 * @struct RaycastCallback
 * @brief Custom callback for handling raycast hits.
 */
struct TOSSENGINE_API RaycastCallback : public reactphysics3d::RaycastCallback
{
    bool hit = false;
    Vector3 point;
    Vector3 normal;
    void* collider = nullptr;
    void* rigidbody = nullptr;

    virtual reactphysics3d::decimal notifyRaycastHit(const RaycastInfo& info) override;
};

/**
 * @struct RaycastDebugEntry
 * @brief Represents a debug line drawn for raycast visualization.
 */
struct TOSSENGINE_API RaycastDebugEntry {
    Vector3 origin;
    Vector3 endPoint;
    float lifetime = 1.0f;
};

/**
 * @class Physics
 * @brief Singleton class that manages the physics world, updates simulations,
 *        handles collisions, triggers, raycasts, and provides debug drawing utilities.
 */
class TOSSENGINE_API Physics : public rp3d::EventListener
{
public:
    /**
     * @brief Gets the singleton instance of the Physics system.
     */
    static Physics& GetInstance();

    /**
     * @brief Updates the internal physics simulation (fixed timestep).
     */
    void UpdateInternal();

    /**
     * @brief Updates the physics system if not paused.
     */
    void Update();

    /**
     * @brief Cleans up the physics system and destroys worlds.
     */
    void CleanUp();

    /**
     * @brief Returns the main physics world.
     */
    PhysicsWorld* GetWorld() const { return m_world; }

    /**
     * @brief Returns the physics world used for prefabs (not actively simulated).
     */
    PhysicsWorld* GetPrefabWorld() const { return m_prefabWorld; }

    /**
     * @brief Accessor for the PhysicsCommon settings object.
     */
    PhysicsCommon& GetPhysicsCommon() { return m_commonSettings; }

    /**
     * @brief Const accessor for PhysicsCommon.
     */
    const PhysicsCommon& GetPhysicsCommon() const { return m_commonSettings; }

    /**
     * @brief Returns the default physics material.
     */
    PhysicsMaterialPtr GetDefaultPhysicsMaterial();

    /**
     * @brief Enables or disables physics debug drawing.
     * @param debug True to enable debug visuals.
     */
    void SetDebug(bool debug);

    /**
     * @brief Pauses or unpauses the physics simulation.
     * @param paused True to pause simulation.
     */
    void SetPaused(bool paused);

    /**
     * @brief Returns whether debug drawing is enabled.
     */
    bool GetDebug() const;

    /**
     * @brief Sets the gravity vector for the physics world.
     * @param gravity The new gravity vector.
     */
    void SetGravity(const Vector3& gravity);

    /**
     * @brief Gets the current gravity vector.
     */
    Vector3 GetGravity() const { return m_gravity; }

    /**
     * @brief Performs a raycast in the world and returns hit information.
     * @param origin The start position of the ray.
     * @param direction The direction of the ray (should be normalized).
     * @param maxDistance Maximum distance to check.
     * @param hitLayers Layer mask for what to hit.
     * @return Result of the raycast.
     */
    RaycastHit Raycast(const Vector3& origin, const Vector3& direction, float maxDistance = 1000.0f, LayerBit hitLayers = 65535);

    /**
     * @brief Draws debug visuals for the physics world.
     * @param data Uniform data for rendering (e.g., camera matrices).
     */
    void DrawDebug(UniformData data) const;

    // ReactPhysics3D callbacks
    void onContact(const rp3d::CollisionCallback::CallbackData& data) override;
    void onTrigger(const rp3d::OverlapCallback::CallbackData& data) override;

    /**
     * @brief Loads and initializes the main physics world.
     */
    void LoadWorld();

    /**
     * @brief Destroys and cleans up the main physics world.
     */
    void UnLoadWorld();

    /**
     * @brief Loads the prefab simulation world.
     */
    void LoadPrefabWorld();

    /**
     * @brief Unloads and destroys the prefab simulation world.
     */
    void UnLoadPrefabWorld();

    /**
     * @brief Safely destroys a collision shape and schedules it for cleanup.
     * @param shape Pointer to the collision shape to destroy.
     */
    void DestroyShape(rp3d::CollisionShape* shape);

private:
    Physics() = default;
    ~Physics() override = default;

private:
    PhysicsCommon m_commonSettings; //!< Settings and factory methods for physics objects.
    bool isDebug = false; //!< Debug drawing toggle.
    PhysicsWorld* m_world = nullptr; //!< Active physics simulation world.
    PhysicsWorld* m_prefabWorld = nullptr; //!< World used for prefab simulation.
    PhysicsMaterialPtr m_defaultPhysicsMaterial = nullptr; //!< Default material used if none is assigned.
    Vector3 m_gravity = Vector3(0.0f, -9.81f, 0.0f); //!< Current gravity applied to the world.

    uint VBO_lines = 0; //!< Vertex Buffer Object ID for debug lines.
    uint VAO_lines = 0; //!< Vertex Array Object ID for debug lines.
    uint VBO_tris = 0; //!< Vertex Buffer Object ID for debug triangles.
    uint VAO_tris = 0; //!< Vertex Array Object ID for debug triangles.

    std::vector<RaycastDebugEntry> m_raycastDebugEntries; //!< Active raycast debug visuals.

    std::vector<rp3d::CollisionShape*> m_shapesToDestroy; //!< Shapes scheduled for destruction.

    bool isPaused = false; //!< Is the physics simulation paused.
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif