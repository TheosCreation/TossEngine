#include "Collider.h"
#include "GameObject.h"
#include "Rigidbody.h"

Collider::~Collider()
{
    if (m_Collider) {
        m_Collider = nullptr;
    }
}

json Collider::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this)); // Store the component type

    if (m_Shape)
    {
        data["colliderType"] = static_cast<int>(m_Shape->getType());

        if (m_Shape->getType() == rp3d::CollisionShapeType::SPHERE)
        {
            rp3d::SphereShape* sphereShape = static_cast<rp3d::SphereShape*>(m_Shape);
            data["radius"] = m_radius;
        }
        else if (m_Shape->getType() == rp3d::CollisionShapeType::CAPSULE)
        {
            rp3d::CapsuleShape* capsuleShape = static_cast<rp3d::CapsuleShape*>(m_Shape);
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
    // Ensure "colliderType" is valid
    if (data.contains("colliderType") && !data["colliderType"].is_null())
    {
        int colliderType = data["colliderType"].get<int>();

        // Ensure the collider type is valid
        if (colliderType < 0 || colliderType > 2) return;

        // Check collider type and deserialize accordingly
        if (colliderType == 0 && data.contains("radius") && !data["radius"].is_null())
        {
            SetSphereCollider(data["radius"].get<float>());
        }
        else if (colliderType == 1 && data.contains("radius") && data.contains("height") &&
            !data["radius"].is_null() && !data["height"].is_null())
        {
            SetCapsuleCollider(data["radius"].get<float>(), data["height"].get<float>());
        }
        else if (colliderType == 2 && data.contains("size") && !data["size"].is_null())
        {
            auto size = data["size"];
            if (size.size() == 3)
            {
                SetBoxCollider(Vector3(size[0], size[1], size[2]));
            }
        }
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

            if (ImGui::DragFloat3("Size", &size.x, 0.1f, 0.1f, 10.0f))
            {
                SetBoxCollider(size);
            }
            break;
        }

        case rp3d::CollisionShapeType::SPHERE:
        {
            float radius = m_radius;

            if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.1f, 10.0f))
            {
                SetSphereCollider(radius);
            }
            break;
        }

        case rp3d::CollisionShapeType::CAPSULE:
        {
            float radius = m_radius;
            float height = m_height;

            if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.1f, 10.0f) ||
                ImGui::DragFloat("Height", &height, 0.1f, 0.1f, 10.0f))
            {
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
    m_Rigidbody = m_owner->getComponent<Rigidbody>();
    // If Rigidbody is not found. this is the only way ive found to be able to get collisions to work
    if (!m_Rigidbody) {
        m_Rigidbody = m_owner->addComponent<Rigidbody>();
        m_Rigidbody->SetBodyType(BodyType::STATIC);
    }
}

void Collider::onCreate()
{
    SetBoxCollider(Vector3(1.0f, 1.0f, 1.0f)); // Default to Box Collider
}

void Collider::SetBoxCollider(const Vector3& size)
{
    m_boxColliderSize = size;

    // Apply transform scale to the size
    Vector3 scaledSize = m_boxColliderSize * m_owner->m_transform.localScale;
    m_Shape = Physics::GetInstance().GetPhysicsCommon().createBoxShape(scaledSize);
}

void Collider::SetSphereCollider(float radius)
{
    m_radius = radius;

    // Apply transform scale to the radius
    float scaledRadius = radius * m_owner->m_transform.localScale.x;
    m_Shape = Physics::GetInstance().GetPhysicsCommon().createSphereShape(scaledRadius);
}

void Collider::SetCapsuleCollider(float radius, float height)
{
    m_radius = radius;
    m_height = height;
    // Apply transform scale to the radius and height
    float scaledRadius = m_radius * m_owner->m_transform.localScale.x;
    float scaledHeight = m_height * m_owner->m_transform.localScale.y;
    m_Shape = Physics::GetInstance().GetPhysicsCommon().createCapsuleShape(scaledRadius, scaledHeight);
}

void Collider::RemoveCollider()
{
    if (m_Collider && m_Rigidbody)
    {
        m_Rigidbody->GetBody()->removeCollider(m_Collider);
        m_Collider = nullptr;
    }
}

rp3d::CollisionShapeType Collider::GetColliderType() const
{
    return m_Shape ? m_Shape->getType() : rp3d::CollisionShapeType::CONVEX_POLYHEDRON;
}