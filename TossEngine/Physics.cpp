#include "Physics.h"
#include "Vector3.h"
#include "ResourceManager.h"
#include "PhysicsMaterial.h"
#include "Rigidbody.h"
#include "Collider.h"
#include "GraphicsEngine.h"
#include "Shader.h"

reactphysics3d::decimal RaycastCallback::notifyRaycastHit(const RaycastInfo& info)
{
    hit = true;
    point = info.worldPoint;
    normal = info.worldNormal;
    collider = info.collider->getUserData();
    rigidbody = info.body->getUserData();
    
    return info.hitFraction;
}

void Physics::Update(float deltaTime)
{
    if (m_world)
    {
        m_world->update(deltaTime);
    }

    // Update raycast debug entries lifetime
    for (auto it = m_raycastDebugEntries.begin(); it != m_raycastDebugEntries.end(); )
    {
        it->lifetime -= deltaTime;
        if (it->lifetime <= 0)
            it = m_raycastDebugEntries.erase(it);
        else
            ++it;
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

void Physics::SetDebug(bool debug)
{
    isDebug = debug;



    debugShader = ResourceManager::GetInstance().getShader("PhysicsDebug");

    glGenVertexArrays(1, &VAO_lines);
    glGenBuffers(1, &VBO_lines);

    glGenVertexArrays(1, &VAO_tris);
    glGenBuffers(1, &VBO_tris);

    // Setup for lines VAO
    glBindVertexArray(VAO_lines);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_lines);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, color));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Setup for triangles VAO
    glBindVertexArray(VAO_tris);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_tris);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, color));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

bool Physics::GetDebug()
{
    return isDebug;
}

void Physics::SetGravity(const Vector3& gravity)
{
    m_gravity = gravity;

    if (m_world)
    {
        m_world->setGravity(static_cast<rp3d::Vector3>(m_gravity));
    }
}

void Physics::DrawDebug(UniformData data)
{
    // Retrieve debug data from ReactPhysics3D
    reactphysics3d::DebugRenderer& debugRenderer = m_world->getDebugRenderer();
    const auto& lines = debugRenderer.getLines();
    const auto& triangles = debugRenderer.getTriangles();

    // Prepare vertex vectors for lines and triangles
    std::vector<DebugVertex> lineVertices;
    for (const auto& line : lines) {
        // First vertex of the line
        DebugVertex v1;
        v1.position = glm::vec3(line.point1.x, line.point1.y, line.point1.z);
        v1.color = glm::vec3(((line.color1 >> 16) & 0xFF) / 255.0f,
            ((line.color1 >> 8) & 0xFF) / 255.0f,
            ((line.color1) & 0xFF) / 255.0f);

        // Second vertex of the line
        DebugVertex v2;
        v2.position = glm::vec3(line.point2.x, line.point2.y, line.point2.z);
        v2.color = glm::vec3(((line.color2 >> 16) & 0xFF) / 255.0f,
            ((line.color2 >> 8) & 0xFF) / 255.0f,
            ((line.color2) & 0xFF) / 255.0f);

        lineVertices.push_back(v1);
        lineVertices.push_back(v2);
    }

    // Add raycast debug lines (using a distinct color, e.g., green)
    for (const auto& ray : m_raycastDebugEntries) {
        DebugVertex v1, v2;
        v1.position = glm::vec3(ray.origin.x, ray.origin.y, ray.origin.z);
        v1.color = glm::vec3(0.0f, 1.0f, 0.0f);  // green color

        v2.position = glm::vec3(ray.endPoint.x, ray.endPoint.y, ray.endPoint.z);
        v2.color = glm::vec3(0.0f, 1.0f, 0.0f);  // green color

        lineVertices.push_back(v1);
        lineVertices.push_back(v2);
    }

    std::vector<DebugVertex> triVertices;
    for (const auto& tri : triangles) {
        DebugVertex v1, v2, v3;
        v1.position = glm::vec3(tri.point1.x, tri.point1.y, tri.point1.z);
        v1.color = glm::vec3(((tri.color1 >> 16) & 0xFF) / 255.0f,
            ((tri.color1 >> 8) & 0xFF) / 255.0f,
            ((tri.color1) & 0xFF) / 255.0f);

        v2.position = glm::vec3(tri.point2.x, tri.point2.y, tri.point2.z);
        v2.color = glm::vec3(((tri.color2 >> 16) & 0xFF) / 255.0f,
            ((tri.color2 >> 8) & 0xFF) / 255.0f,
            ((tri.color2) & 0xFF) / 255.0f);

        v3.position = glm::vec3(tri.point3.x, tri.point3.y, tri.point3.z);
        v3.color = glm::vec3(((tri.color3 >> 16) & 0xFF) / 255.0f,
            ((tri.color3 >> 8) & 0xFF) / 255.0f,
            ((tri.color3) & 0xFF) / 255.0f);

        triVertices.push_back(v1);
        triVertices.push_back(v2);
        triVertices.push_back(v3);
    }

    // Update the VBO for lines
    glBindBuffer(GL_ARRAY_BUFFER, VBO_lines);
    glBufferData(GL_ARRAY_BUFFER, lineVertices.size() * sizeof(DebugVertex), lineVertices.data(), GL_DYNAMIC_DRAW);

    // Update the VBO for triangles
    glBindBuffer(GL_ARRAY_BUFFER, VBO_tris);
    glBufferData(GL_ARRAY_BUFFER, triVertices.size() * sizeof(DebugVertex), triVertices.data(), GL_DYNAMIC_DRAW);

    // Set the shader and VP matrix
    GraphicsEngine::GetInstance().setShader(debugShader);
    debugShader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);

    glDisable(GL_DEPTH_TEST);

    // Draw lines (including raycasts)
    glBindVertexArray(VAO_lines);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lineVertices.size()));

    // Draw collider wireframes
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(VAO_tris);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(triVertices.size()));

    // Re-enable depth testing and restore polygon fill mode
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Clean up
    glBindVertexArray(0);
    glUseProgram(0);

}

RaycastHit Physics::Raycast(const Vector3& origin, const Vector3& direction, float maxDistance, LayerBit hitLayers)
{
    Ray ray(static_cast<rp3d::Vector3>(origin), static_cast<rp3d::Vector3>(direction));

    RaycastHit hitResult;
    RaycastCallback callback;

    m_world->raycast(ray, &callback, hitLayers);

    // If a hit occurred, compute the distance.
    if (callback.hit)
    {
        float distance = Vector3::Distance(origin, Vector3(callback.point));
        // If the hit is farther than maxDistance, treat it as a miss.
        if (distance > maxDistance)
        {
            callback.hit = false;
        }
    }

    // Populate the hit result.
    hitResult.hasHit = callback.hit;
    if (callback.hit)
    {
        hitResult.point = Vector3(callback.point);
        hitResult.normal = Vector3(callback.normal);
        hitResult.distance = Vector3::Distance(origin, hitResult.point);
        hitResult.collider = static_cast<Collider*>(callback.collider);
        hitResult.rigidbody = static_cast<Rigidbody*>(callback.rigidbody);
    }
    else
    {
        // No valid hit within maxDistance.
        hitResult.distance = maxDistance;
    }

    // Debug visualization.
    if (isDebug)
    {
        Vector3 endPoint = hitResult.hasHit ? hitResult.point : (origin + direction * maxDistance);
        m_raycastDebugEntries.push_back({ origin, endPoint });
    }

    return hitResult;
}

void Physics::onContact(const rp3d::CollisionCallback::CallbackData& data) {
    // Build a set for current active collision pairs.
    std::unordered_set<CollisionPair> currentCollisionPairs;

    int nbContactPairs = data.getNbContactPairs();
    if (nbContactPairs == 0) {
        return;
    }

    // Process each contact pair.
    for (int i = 0; i < nbContactPairs; i++) {
        const rp3d::CollisionCallback::ContactPair& contactPair = data.getContactPair(i);
        int nbContactPoints = contactPair.getNbContactPoints();

        // If no contact points, skip processing this pair.
        if (nbContactPoints == 0) {
            continue;
        }

        // Retrieve the colliders.
        rp3d::Collider* collider1 = contactPair.getCollider1();
        rp3d::Collider* collider2 = contactPair.getCollider2();

        // Create a CollisionPair and add it to the current set.
        CollisionPair currentPair{ collider1, collider2 };
        currentCollisionPairs.insert(currentPair);

        // Get the bodies from the colliders.
        rp3d::Body* body1 = collider1->getBody();
        rp3d::Body* body2 = collider2->getBody();

        if (body1 && body2) {
            // Retrieve the custom Rigidbody components.
            Rigidbody* customRigidbody1 = static_cast<Rigidbody*>(body1->getUserData());
            Rigidbody* customRigidbody2 = static_cast<Rigidbody*>(body2->getUserData());

            // Only call OnCollisionEnter if this collision pair is new.
            if (m_previousCollisionPairs.find(currentPair) == m_previousCollisionPairs.end()) {
                if (customRigidbody1) {
                    customRigidbody1->OnCollisionEnter(customRigidbody2);
                }
                if (customRigidbody2) {
                    customRigidbody2->OnCollisionEnter(customRigidbody1);
                }
            }
        }
        else {
            std::cerr << "Invalid body detected in contact pair." << std::endl;
        }
    }

    // Detect collision exit events:
    // For any pair that was active in the previous frame but not in the current one, call OnCollisionExit.
    for (const auto& previousPair : m_previousCollisionPairs) {
        if (currentCollisionPairs.find(previousPair) == currentCollisionPairs.end()) {
            rp3d::Collider* collider1 = previousPair.collider1;
            rp3d::Collider* collider2 = previousPair.collider2;

            rp3d::Body* body1 = collider1->getBody();
            rp3d::Body* body2 = collider2->getBody();

            if (body1 && body2) {
                Rigidbody* customRigidbody1 = static_cast<Rigidbody*>(body1->getUserData());
                Rigidbody* customRigidbody2 = static_cast<Rigidbody*>(body2->getUserData());

                if (customRigidbody1) {
                    customRigidbody1->OnCollisionExit(customRigidbody2);
                }
                if (customRigidbody2) {
                    customRigidbody2->OnCollisionExit(customRigidbody1);
                }
            }
        }
    }

    // Update the stored collision pairs for the next simulation step.
    m_previousCollisionPairs = currentCollisionPairs;
}

void Physics::onTrigger(const rp3d::OverlapCallback::CallbackData& data) {
    // Create a set to hold the current trigger pairs.
    std::unordered_set<TriggerPair> currentTriggerPairs;

    int nbOverlapPairs = data.getNbOverlappingPairs();
    for (int i = 0; i < nbOverlapPairs; i++) {
        const rp3d::OverlapCallback::OverlapPair& overlapPair = data.getOverlappingPair(i);
        rp3d::Collider* collider1 = overlapPair.getCollider1();
        rp3d::Collider* collider2 = overlapPair.getCollider2();

        // Construct the trigger pair.
        TriggerPair pair{ collider1, collider2 };
        currentTriggerPairs.insert(pair);

        // If this pair was not present last frame, it's a new trigger event.
        if (m_previousTriggerPairs.find(pair) == m_previousTriggerPairs.end()) {
            rp3d::Body* body1 = collider1->getBody();
            rp3d::Body* body2 = collider2->getBody();

            if (body1 && body2) {
                Rigidbody* customRigidbody1 = static_cast<Rigidbody*>(body1->getUserData());
                Rigidbody* customRigidbody2 = static_cast<Rigidbody*>(body2->getUserData());

                if (customRigidbody1 && collider1->getIsTrigger()) {
                    customRigidbody1->OnTriggerEnter(customRigidbody2);
                }
                if (customRigidbody2 && collider2->getIsTrigger()) {
                    customRigidbody2->OnTriggerEnter(customRigidbody1);
                }
            }
        }
    }

    // Now check for pairs that were present last frame but are not in the current frame.
    for (const auto& pair : m_previousTriggerPairs) {
        if (currentTriggerPairs.find(pair) == currentTriggerPairs.end()) {
            rp3d::Collider* collider1 = pair.collider1;
            rp3d::Collider* collider2 = pair.collider2;
            rp3d::Body* body1 = collider1->getBody();
            rp3d::Body* body2 = collider2->getBody();

            if (body1 && body2) {
                Rigidbody* customRigidbody1 = static_cast<Rigidbody*>(body1->getUserData());
                Rigidbody* customRigidbody2 = static_cast<Rigidbody*>(body2->getUserData());

                if (customRigidbody1 && collider1->getIsTrigger()) {
                    customRigidbody1->OnTriggerExit(customRigidbody2);
                }
                if (customRigidbody2 && collider2->getIsTrigger()) {
                    customRigidbody2->OnTriggerExit(customRigidbody1);
                }
            }
        }
    }

    // Update the stored trigger pairs for the next simulation step.
    m_previousTriggerPairs = currentTriggerPairs;
}

void Physics::LoadWorld()
{
    PhysicsMaterialDesc physicsMaterialDesc; //full defaults set in the code atm
    m_defaultPhysicsMaterial = std::make_shared<PhysicsMaterial>(physicsMaterialDesc, "DefaultPhysicsMaterial", nullptr);

    rp3d::PhysicsWorld::WorldSettings settings;
    settings.gravity = static_cast<rp3d::Vector3>(m_gravity);
    settings.defaultPositionSolverNbIterations = 20;  // Default is usually 6
    settings.defaultVelocitySolverNbIterations = 10;  // Default is usually 4
    m_world = m_commonSettings.createPhysicsWorld(settings);
    m_world->setEventListener(this);


    m_world->setIsDebugRenderingEnabled(isDebug);

    // Get a reference to the debug renderer
    reactphysics3d::DebugRenderer& debugRenderer = m_world->getDebugRenderer();

    // Select the contact points and contact normals to be displayed
    debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::CONTACT_POINT, isDebug);
    debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::CONTACT_NORMAL, isDebug);
    debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::COLLISION_SHAPE, isDebug);
    debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::COLLIDER_AABB, isDebug);
}

void Physics::UnLoadWorld()
{
    if (m_world)
    {
        m_commonSettings.destroyPhysicsWorld(m_world);
        m_world = nullptr;
        m_raycastDebugEntries.clear();

        m_previousTriggerPairs.clear();
        m_previousCollisionPairs.clear();
    }
}
