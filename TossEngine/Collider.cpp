#include "Collider.h"
#include "GameObject.h"
#include "Rigidbody.h"
#include "PhysicsMaterial.h"

json Collider::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this)); // Store the component type

    if (m_physicsMaterial)
    {
        data["physicsMaterial"] = m_physicsMaterial->getUniqueID();
    }

    data["layerNames"] = m_layerNames;

    data["isTrigger"] = m_isTrigger;

    if (m_Shape)
    {
        data["colliderType"] = static_cast<int>(m_Shape->getType());

        if (m_Shape->getType() == rp3d::CollisionShapeType::SPHERE)
        {
            data["radius"] = m_radius;
        }
        else if (m_Shape->getType() == rp3d::CollisionShapeType::CAPSULE)
        {
            data["radius"] = m_radius;
            data["height"] = m_height;
        }
        else if (m_Shape->getType() == rp3d::CollisionShapeType::CONVEX_POLYHEDRON)
        {
            data["size"] = { m_boxColliderSize.x , m_boxColliderSize.y, m_boxColliderSize.z }; // Convert to full extents
        }
    }

    return data;
}

void Collider::deserialize(const json& data)
{
    if (data.contains("physicsMaterial"))
    {
        string id = data["physicsMaterial"].get<string>();
        if (!id.empty())
        {
            m_physicsMaterial = ResourceManager::GetInstance().getPhysicsMaterial(id);
        }
    }

    if (data.contains("layerNames")) {
        m_layerNames = data["layerNames"].get<std::vector<std::string>>();
        if (m_Rigidbody != nullptr) {
            UpdateRP3Collider();
        }
    }

    if (data.contains("isTrigger"))
    {
        m_isTrigger = data["isTrigger"];
    }

    if (data.contains("colliderType") && !data["colliderType"].is_null())
    {
        int colliderType = data["colliderType"].get<int>();
        if (data.contains("radius") && !data["radius"].is_null())
        {
            m_radius = data["radius"].get<float>();
        }
        if (data.contains("height") && !data["height"].is_null())
        {
            m_height = data["height"].get<float>();
        }
        if (data.contains("size") && !data["size"].is_null())
        {
            auto size = data["size"];
            if (size.size() == 3)
            {
                m_boxColliderSize = Vector3(size[0], size[1], size[2]);
            }
        }
        SetColliderType(colliderType);
    }
    else
    {
        // Handle missing or null colliderType gracefully
        SetBoxCollider(Vector3(1.0f, 1.0f, 1.0f)); // Default to a box collider if data is invalid
    }
}

void Collider::OnInspectorGUI()
{
    ImGui::Text("Collider Inspector - ID: %p", this);
    ImGui::Separator();

    if(ImGui::Checkbox("Is Trigger", &m_isTrigger))
    {
        if (m_isTrigger == true)
        {
            if (m_Rigidbody)
            {
                m_Rigidbody->SetBodyType(BodyType::KINEMATIC); //make the body type kinematic to make trigger collider work
            }
        }
    }

    ResourceAssignableField(m_physicsMaterial, "Physics Material");

    // Get available layers from the LayerManager.
    LayerManager& layerManager = LayerManager::GetInstance();
    const auto& layers = layerManager.GetLayers();

    // Build a vector copy of available layer names.
    std::vector<std::string> layerNames;
    for (const auto& pair : layers) {
        layerNames.push_back(pair.first);
    }

    if (LayerDropdownField("Include Layers", m_layerNames))
    {
        if (m_Rigidbody != nullptr) {
            UpdateRP3Collider();
        }
    }
    ImGui::Separator();

    // List of shape types for the dropdown menu
    static const char* shapeTypes[] = { "Sphere", "Capsule", "Box" };
    int currentShapeType = static_cast<int>(m_Shape ? m_Shape->getType() : rp3d::CollisionShapeType::CONVEX_POLYHEDRON);

    // Dropdown menu for selecting the collider type
    if (ImGui::Combo("Collider Type", &currentShapeType, shapeTypes, IM_ARRAYSIZE(shapeTypes)))
    {
        // If the user changes the shape, recreate it
        switch (currentShapeType)
        {
        case static_cast<int>(rp3d::CollisionShapeType::CONVEX_POLYHEDRON):
            SetBoxCollider(Vector3(1.0f, 1.0f, 1.0f)); // Default size
            break;
        case static_cast<int>(rp3d::CollisionShapeType::SPHERE):
            SetSphereCollider(0.5f); // Default radius
            break;
        case static_cast<int>(rp3d::CollisionShapeType::CAPSULE):
            SetCapsuleCollider(0.5f, 1.0f); // Default radius and height
            break;
        }
    }

    // Display relevant variables for the selected shape
    if (m_Shape)
    {
        switch (m_Shape->getType())
        {
        case rp3d::CollisionShapeType::CONVEX_POLYHEDRON:
        {
            Vector3 size = m_boxColliderSize;
            // drag with a 0.01f minimum
            if (ImGui::DragFloat3("Size", &size.x, 0.1f, 0.01f, 10.0f))
            {
                // enforce strictly positive
                size.x = std::max(size.x, 0.01f);
                size.y = std::max(size.y, 0.01f);
                size.z = std::max(size.z, 0.01f);
                SetBoxCollider(size);
            }
            break;
        }

        case rp3d::CollisionShapeType::SPHERE:
        {
            float radius = m_radius;
            // drag with a 0.01f minimum
            if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.01f, 10.0f))
            {
                radius = std::max(radius, 0.01f);
                SetSphereCollider(radius);
            }
            break;
        }

        case rp3d::CollisionShapeType::CAPSULE:
        {
            float radius = m_radius;
            float height = m_height;
            // both fields have a 0.01f floor
            if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.01f, 10.0f) ||
                ImGui::DragFloat("Height", &height, 0.1f, 0.01f, 10.0f))
            {
                radius = std::max(radius, 0.01f);
                height = std::max(height, 0.01f);
                SetCapsuleCollider(radius, height);
            }
            break;
        }

        default:
            ImGui::Text("Unknown Shape Type");
            break;
        }
    }
}


void Collider::onStart()
{
    //collider->setIsTrigger(true);
    // Set the material properties
    rp3d::Material colliderMaterial = m_Collider->getMaterial();
    colliderMaterial.setFrictionCoefficient(m_physicsMaterial->getDynamicFriction());
    colliderMaterial.setBounciness(m_physicsMaterial->getBounciness());

    // Apply the updated material to the collider
    m_Collider->setMaterial(colliderMaterial);
}

void Collider::onCreate()
{
}

void Collider::onCreateLate()
{
    m_Rigidbody = m_owner->getComponent<Rigidbody>();
    if (m_Rigidbody == nullptr) {
        m_Rigidbody = m_owner->addComponent<Rigidbody>();
        m_Rigidbody->SetBodyType(BodyType::STATIC);
    }

    if (m_physicsMaterial == nullptr)
    {
        m_physicsMaterial = Physics::GetInstance().GetDefaultPhysicsMaterial();
    }

    if (m_Shape == nullptr)
    {
        SetBoxCollider(Vector3(1.0f, 1.0f, 1.0f)); // Default to Box Collider
    }
    else
    {
        UpdateRP3Collider();
    }
}

void Collider::onRescale(const Vector3& previousScale)
{
    //update the physics shape based on the scale
    if (m_owner->m_transform.GetLocalScale().Length() > 0)
    {
        SetColliderType(static_cast<int>(m_Shape->getType()));
    }
}

void Collider::SetColliderType(int type)
{
    switch (type) {
        case (static_cast<int>(rp3d::CollisionShapeType::SPHERE)) :
            // re‑use stored radius
            SetSphereCollider(m_radius);
            break;
            case static_cast<int>(rp3d::CollisionShapeType::CAPSULE) :
                // re‑use stored radius & height
                SetCapsuleCollider(m_radius, m_height);
                break;
                case static_cast<int>(rp3d::CollisionShapeType::CONVEX_POLYHEDRON) :
                    // re‑use stored size
                    SetBoxCollider(m_boxColliderSize);
                    break;
                default:
                    Debug::LogError("Collider::SetColliderType(): invalid type ", type);
    }
}

void Collider::SetBoxCollider(const Vector3& size) {
    m_boxColliderSize = size;
    
    Vector3 scale = m_owner->m_transform.GetLocalScale();
    float sx = std::fabs(scale.x), sy = std::fabs(scale.y), sz = std::fabs(scale.z);
    Vector3 scaledSize{ m_boxColliderSize.x * sx,
                               m_boxColliderSize.y * sy,
                               m_boxColliderSize.z * sz };
    // 3) Create the box with those half‑extents
    m_Shape = Physics::GetInstance()
        .GetPhysicsCommon()
        .createBoxShape(scaledSize);
    if (m_Rigidbody) UpdateRP3Collider();
}

void Collider::SetSphereCollider(float radius) {
    m_radius = radius;
    // Pick the largest axis so the shape stays a true sphere
    Vector3 s = m_owner->m_transform.GetLocalScale();
    float maxScale = std::max({ std::fabs(s.x),
                                std::fabs(s.y),
                                std::fabs(s.z) });
    float scaledRadius = radius * maxScale;
    m_Shape = Physics::GetInstance()
        .GetPhysicsCommon()
        .createSphereShape(scaledRadius);
    if (m_Rigidbody) UpdateRP3Collider();
}

void Collider::SetCapsuleCollider(float radius, float height) {
    m_radius = radius;
    m_height = height;
    Vector3 s = m_owner->m_transform.GetLocalScale();
    // Radius in the XZ‑plane, height along Y
    float radiusScale = std::max(std::fabs(s.x), std::fabs(s.z));
    float scaledRadius = radius * radiusScale;
    float scaledHeight = height * std::fabs(s.y);
    m_Shape = Physics::GetInstance()
        .GetPhysicsCommon()
        .createCapsuleShape(scaledRadius, scaledHeight);
    if (m_Rigidbody) UpdateRP3Collider();
}

void Collider::UpdateRP3Collider()
{
    rp3d::RigidBody* rb = m_Rigidbody->GetBody();
    if (m_Collider != nullptr) {
        rb->removeCollider(m_Collider);
        m_Collider = nullptr;
    }

    m_Collider = rb->addCollider(m_Shape, rp3d::Transform::identity());
    m_Collider->setIsTrigger(m_isTrigger);
    m_Collider->setUserData(this);

    // Ensure there is always at least the "Default" layer.
    if (m_layerNames.empty()) {
        m_layerNames.push_back("Default");
    }

    LayerManager& layerManager = LayerManager::GetInstance();
    unsigned short maskBits = 0;
    for (const auto& layerName : m_layerNames) {
        maskBits |= static_cast<unsigned short>(layerManager.GetLayer(layerName));
    }

    // Use the owner's layer as the category bits.
    m_Collider->setCollisionCategoryBits(m_owner->getLayer());
    // Set the collision mask bits based on the selected layers.
    m_Collider->setCollideWithMaskBits(maskBits);
}


void Collider::SetTrigger(bool trigger)
{
    m_isTrigger = trigger;
}

bool Collider::GetTrigger() const
{
    return m_isTrigger;
}

void Collider::OnTriggerEnter(Collider* otherCollider) const
{
    m_owner->CallOnTriggerEnterCallbacks(otherCollider);
}

void Collider::OnTriggerExit(Collider* otherCollider) const
{
    m_owner->CallOnTriggerExitCallbacks(otherCollider);
}

rp3d::CollisionShapeType Collider::GetColliderType() const
{
    return m_Shape ? m_Shape->getType() : rp3d::CollisionShapeType::CONVEX_POLYHEDRON;
}

PhysicsMaterialPtr Collider::GetPhysicsMaterial() const
{
    return m_physicsMaterial;
}
