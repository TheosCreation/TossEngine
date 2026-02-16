#include "Physics.h"
#include "Vector3.h"
#include "ResourceManager.h"
#include "PhysicsMaterial.h"
#include "Rigidbody.h"
#include "Collider.h"
#include "GraphicsEngine.h"
#include "Shader.h"
#include "DebugDraw.h"

reactphysics3d::decimal RaycastCallback::notifyRaycastHit(const RaycastInfo& info)
{
    hit = true;
    point = info.worldPoint;
    normal = info.worldNormal;
    collider = info.collider->getUserData();
    rigidbody = info.body->getUserData();
    
    return info.hitFraction;
}

Physics& Physics::GetInstance()
{
    static Physics instance;
    return instance;
}

void Physics::UpdateInternal()
{
    for (rp3d::CollisionShape* shape : m_shapesToDestroy)
    {
        switch (shape->getType()) {
        case rp3d::CollisionShapeType::CONVEX_POLYHEDRON:
            m_commonSettings.destroyBoxShape(dynamic_cast<rp3d::BoxShape*>(shape));
            break;
        case rp3d::CollisionShapeType::SPHERE:
            m_commonSettings.destroySphereShape(dynamic_cast<rp3d::SphereShape*>(shape));
            break;
        case rp3d::CollisionShapeType::CAPSULE:
            m_commonSettings.destroyCapsuleShape(dynamic_cast<rp3d::CapsuleShape*>(shape));
            break;
        case reactphysics3d::CollisionShapeType::CONCAVE_SHAPE:
            // TODO: Add mesh collider
            break;
        }

    }
    m_shapesToDestroy.clear();
}

void Physics::Update()
{
    if (isDebug)
    {
        // Update raycast debug entries lifetime
        for (auto it = m_raycastDebugEntries.begin(); it != m_raycastDebugEntries.end(); )
        {
            it->lifetime -= Time::FixedDeltaTime;
            if (it->lifetime <= 0)
                it = m_raycastDebugEntries.erase(it);
            else
                ++it;
        }
    }

    //if (m_prefabWorld)
    //{
    //    m_prefabWorld->update(Time::FixedDeltaTime);
    //}
    if (isPaused) return;

    if (Time::FixedDeltaTime > 0.0f && m_world)
    {
        m_world->update(Time::FixedDeltaTime);
    }
}

void Physics::UpdateEditorWorld()
{
    if (m_editorWorld)
    {
        m_editorWorld->update(Time::FixedDeltaTime);
    }
}

void Physics::CleanUp()
{
    if (m_world)
    {
        m_commonSettings.destroyPhysicsWorld(m_world);
        m_world = nullptr;
    }
    if (m_prefabWorld)
    {
        m_commonSettings.destroyPhysicsWorld(m_prefabWorld);
        m_prefabWorld = nullptr;
    }
    if (m_editorWorld)
    {
        m_commonSettings.destroyPhysicsWorld(m_editorWorld);
        m_editorWorld = nullptr;
    }

    m_shapesToDestroy.clear();
    m_raycastDebugEntries.clear();
}

PhysicsMaterialPtr Physics::GetDefaultPhysicsMaterial()
{
    return m_defaultPhysicsMaterial;
}

void Physics::SetDebug(bool debug)
{
    isDebug = debug;

    glGenVertexArrays(1, &VAO_lines);
    glGenBuffers(1, &VBO_lines);

    glGenVertexArrays(1, &VAO_tris);
    glGenBuffers(1, &VBO_tris);

    // Setup for lines VAO
    glBindVertexArray(VAO_lines);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_lines);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), reinterpret_cast<void*>(offsetof(DebugVertex, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), reinterpret_cast<void*>(offsetof(DebugVertex, color)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Setup for triangles VAO
    glBindVertexArray(VAO_tris);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_tris);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), reinterpret_cast<void*>(offsetof(DebugVertex, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), reinterpret_cast<void*>(offsetof(DebugVertex, color)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void Physics::SetFullDebug(bool debug)
{
    isFullDebug = debug;
}

void Physics::SetPaused(bool paused)
{
    isPaused = paused;
}

bool Physics::GetDebug() const
{
    return isDebug;
}

bool Physics::GetFullDebug() const
{
    return isFullDebug;
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
    DrawWorld(data, m_world);
    DrawWorld(data, m_prefabWorld);
    if (isFullDebug)
    {
        DrawWorld(data, m_editorWorld);
    }
}

void Physics::DrawWorld(UniformData data, PhysicsWorld* worldToDraw)
{
    GraphicsEngine& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setBlendFunc(BlendType::SrcAlpha, BlendType::OneMinusSrcAlpha);
    // Retrieve debug data from ReactPhysics3D
    reactphysics3d::DebugRenderer& debugRenderer = worldToDraw->getDebugRenderer();
    debugRenderer.reset();
    debugRenderer.computeDebugRenderingPrimitives(*worldToDraw);
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

    ShaderPtr shader = DebugDraw::GetInstance().GetShader();
    // Set the shader and VP matrix
    GraphicsEngine::GetInstance().setShader(shader);
    shader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);

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

    graphicsEngine.setBlendFunc(BlendType::Zero, BlendType::Zero);
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

RaycastHit Physics::EditorRaycast(const Vector3& origin, const Vector3& direction, float maxDistance,
    LayerBit hitLayers)
{
    Vector3 dir = direction;
    dir = dir.Normalized();
    const Vector3 end = origin + dir * maxDistance;

    rp3d::Ray ray(static_cast<rp3d::Vector3>(origin),
        static_cast<rp3d::Vector3>(end));

    RaycastHit hitResult;
    RaycastCallback callback;

    m_editorWorld->raycast(ray, &callback, hitLayers);

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

    //if (isDebug)
    //{
    //    Vector3 endPoint = hitResult.hasHit ? hitResult.point : (origin + direction * maxDistance);
    //    m_raycastDebugEntries.push_back({ origin, endPoint });
    //}

    return hitResult;
}

void Physics::LoadWorld()
{
    PhysicsMaterialDesc physicsMaterialDesc; //full defaults set in the code atm
    m_defaultPhysicsMaterial = std::make_shared<PhysicsMaterial>(physicsMaterialDesc, "DefaultPhysicsMaterial", nullptr);

    rp3d::PhysicsWorld::WorldSettings settings;
    settings.worldName = "ScenePhysicsWorld";
    settings.gravity = static_cast<rp3d::Vector3>(m_gravity);
    settings.defaultPositionSolverNbIterations = 10;  // Default is usually 6
    settings.defaultVelocitySolverNbIterations = 6;  // Default is usually 4
    settings.isSleepingEnabled = true;
    m_world = m_commonSettings.createPhysicsWorld(settings);
    m_world->setEventListener(&m_listener);


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
    }
}

void Physics::LoadPrefabWorld()
{
    rp3d::PhysicsWorld::WorldSettings settings;
    settings.worldName = "PrefabPhysicsWorld";
    settings.gravity = static_cast<rp3d::Vector3>(m_gravity);
    settings.defaultPositionSolverNbIterations = 6;
    settings.defaultVelocitySolverNbIterations = 4;
    m_prefabWorld = m_commonSettings.createPhysicsWorld(settings);


    m_prefabWorld->setIsDebugRenderingEnabled(isDebug);

    // Get a reference to the debug renderer
    reactphysics3d::DebugRenderer& debugRenderer = m_prefabWorld->getDebugRenderer();

    // Select the contact points and contact normals to be displayed
    debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::CONTACT_POINT, isDebug);
    debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::CONTACT_NORMAL, isDebug);
    debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::COLLISION_SHAPE, isDebug);
    debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::COLLIDER_AABB, isDebug);
}

void Physics::UnLoadPrefabWorld()
{
    if (m_prefabWorld)
    {
        m_commonSettings.destroyPhysicsWorld(m_prefabWorld);
        m_prefabWorld = nullptr;
    }
}

void Physics::LoadEditorWorld()
{
    rp3d::PhysicsWorld::WorldSettings settings;
    settings.worldName = "EditorPhysicsWorld";
    settings.gravity = static_cast<rp3d::Vector3>(m_gravity);
    settings.defaultPositionSolverNbIterations = 6;
    settings.defaultVelocitySolverNbIterations = 4;
    m_editorWorld = m_commonSettings.createPhysicsWorld(settings);

    m_editorWorld->setIsDebugRenderingEnabled(isDebug);

    // Get a reference to the debug renderer
    reactphysics3d::DebugRenderer& debugRenderer = m_editorWorld->getDebugRenderer();

    // Select the contact points and contact normals to be displayed
    debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::CONTACT_POINT, isDebug);
    debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::CONTACT_NORMAL, isDebug);
    debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::COLLISION_SHAPE, isDebug);
    debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::COLLIDER_AABB, isDebug);
}

void Physics::UnLoadEditorWorld()
{
    if (m_editorWorld)
    {
        m_commonSettings.destroyPhysicsWorld(m_editorWorld);
        m_editorWorld = nullptr;
    }
}

void Physics::DestroyShape(rp3d::CollisionShape* shape)
{
    m_shapesToDestroy.push_back(shape);
}

void Physics::HandleContact(const rp3d::CollisionCallback::CallbackData& data)
{
    const int contactPairCount = static_cast<int>(data.getNbContactPairs());
    if (contactPairCount == 0) {
        return;
    }

    for (int pairIndex = 0; pairIndex < contactPairCount; pairIndex++) {

        const rp3d::CollisionCallback::ContactPair& contactPair = data.getContactPair(pairIndex);
        const rp3d::CollisionCallback::ContactPair::EventType eventType = contactPair.getEventType();

        rp3d::Collider* collider1 = contactPair.getCollider1();
        rp3d::Collider* collider2 = contactPair.getCollider2();

        if (collider1 == nullptr) {
            continue;
        }
        if (collider2 == nullptr) {
            continue;
        }

        Collider* customCollider1 = static_cast<Collider*>(collider1->getUserData());
        Collider* customCollider2 = static_cast<Collider*>(collider2->getUserData());

        if (customCollider1 == nullptr) {
            continue;
        }
        if (customCollider2 == nullptr) {
            continue;
        }

        if (eventType == rp3d::CollisionCallback::ContactPair::EventType::ContactExit) {
            Collision exitForBody1;
            exitForBody1.contactPoint = Vector3(0.0f, 0.0f, 0.0f);
            exitForBody1.contactNormal = Vector3(0.0f, 0.0f, 0.0f);
            exitForBody1.otherCollider = customCollider2;
            customCollider1->CallOnCollisionExitCallbacks(exitForBody1);

            Collision exitForBody2;
            exitForBody2.contactPoint = Vector3(0.0f, 0.0f, 0.0f);
            exitForBody2.contactNormal = Vector3(0.0f, 0.0f, 0.0f);
            exitForBody2.otherCollider = customCollider1;
            customCollider2->CallOnCollisionExitCallbacks(exitForBody2);

            continue;
        }

        const int contactPointCount = static_cast<int>(contactPair.getNbContactPoints());
        if (contactPointCount == 0) {
            continue;
        }

        const rp3d::Transform xform1 = collider1->getLocalToWorldTransform();
        const rp3d::Transform xform2 = collider2->getLocalToWorldTransform();

        for (int cpIndex = 0; cpIndex < contactPointCount; cpIndex++) {

            const rp3d::CollisionCallback::ContactPoint cp = contactPair.getContactPoint(cpIndex);

            const rp3d::Vector3 normalWorldRP3D = cp.getWorldNormal();  // Direction defined by RP3D
            const Vector3 normalTowardBody1 = Vector3(normalWorldRP3D);
            const Vector3 normalTowardBody2 = Vector3(-normalTowardBody1.x, -normalTowardBody1.y, -normalTowardBody1.z);

            const rp3d::Vector3 localOn1 = cp.getLocalPointOnCollider1();
            const rp3d::Vector3 localOn2 = cp.getLocalPointOnCollider2();

            const rp3d::Vector3 worldOn1RP3D = xform1 * localOn1;
            const rp3d::Vector3 worldOn2RP3D = xform2 * localOn2;

            const Vector3 worldOn1 = Vector3(worldOn1RP3D);
            const Vector3 worldOn2 = Vector3(worldOn2RP3D);

            Collision enterForBody1;
            enterForBody1.contactPoint = worldOn1;
            enterForBody1.contactNormal = normalTowardBody1; // Normal points toward body1 per RP3D docs
            enterForBody1.otherCollider = customCollider2;

            Collision enterForBody2;
            enterForBody2.contactPoint = worldOn2;
            enterForBody2.contactNormal = normalTowardBody2;
            enterForBody2.otherCollider = customCollider1;

            if (eventType == rp3d::CollisionCallback::ContactPair::EventType::ContactStart) {
                customCollider1->CallOnCollisionEnterCallbacks(enterForBody1);
                customCollider2->CallOnCollisionEnterCallbacks(enterForBody2);
            }
            else {
                customCollider1->CallOnCollisionStayCallbacks(enterForBody1);
                customCollider2->CallOnCollisionStayCallbacks(enterForBody2);
            }
        }
    }
}

void Physics::HandleTrigger(const rp3d::OverlapCallback::CallbackData& data)
{
    int nbOverlapPairs = data.getNbOverlappingPairs();
    for (int i = 0; i < nbOverlapPairs; i++) {
        const rp3d::OverlapCallback::OverlapPair& overlapPair = data.getOverlappingPair(i);
        auto eventType = overlapPair.getEventType();
        rp3d::Collider* collider1 = overlapPair.getCollider1();
        rp3d::Collider* collider2 = overlapPair.getCollider2();

        if (!collider1 || !collider2) continue;

        Collider* customCollider1 = static_cast<Collider*>(collider1->getUserData());
        Collider* customCollider2 = static_cast<Collider*>(collider2->getUserData());

        if (!customCollider1 || !customCollider2) continue;

        if (eventType == rp3d::OverlapCallback::OverlapPair::EventType::OverlapStart) {
            if (customCollider1->GetTrigger()) {

                customCollider1->CallOnTriggerEnterCallbacks(customCollider2);
            }
            if (customCollider2->GetTrigger()) {

                customCollider2->CallOnTriggerEnterCallbacks(customCollider1);
            }
        }
        else if (eventType == rp3d::OverlapCallback::OverlapPair::EventType::OverlapExit) {
            if (customCollider1->GetTrigger()) {

                customCollider1->CallOnTriggerExitCallbacks(customCollider2);
            }
            if (customCollider2->GetTrigger()) {

                customCollider2->CallOnTriggerExitCallbacks(customCollider1);
            }
        }
    }
}
