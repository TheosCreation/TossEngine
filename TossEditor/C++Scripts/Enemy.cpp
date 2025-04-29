#include "Enemy.h"
#include "PlayerController.h"

void Enemy::onStart()
{
    m_target = m_owner->getGameObjectManager()->findObjectOfType<PlayerController>();
    m_rigidbody = m_owner->getComponent<Rigidbody>();
}

void Enemy::OnInspectorGUI()
{
    Component::OnInspectorGUI();
    IntSliderField("Health", m_health);
    FloatSliderField("Speed", m_speed);
    FloatSliderField("Wall Detection Distance", m_wallDetectDistance);
    
    vector<std::string> layers = m_layerNames;
    if (LayerDropdownField("Raycast hit layers", layers))
    {
        m_layerNames = layers;
    }
}
void Enemy::onFixedUpdate()
{
    if (!m_target || !m_rigidbody) return;

    auto& transform = m_owner->m_transform;

    Vector3 myPos = transform.position;
    Vector3 playerPos = m_target->getOwner()->m_transform.position;
    Vector3 delta = playerPos - myPos;
    delta.y = 0.0f;

    float dist = delta.Length();
    if (dist < 0.1f)
    {
        // close enough: zero horizontal velocity
        glm::vec3 v = m_rigidbody->GetLinearVelocity();
        v.x = v.z = 0.0f;
        m_rigidbody->SetLinearVelocity(v);
        return;
    }

    Vector3 dir = delta / dist; // unit vector toward player

    Vector3 forward = dir; // Forward = direction to player
    forward.Normalize();

    Quaternion yaw45 = Quaternion::FromEuler(Vector3(0, glm::radians(45.0f), 0));
    Quaternion yawM45 = Quaternion::FromEuler(Vector3(0, glm::radians(-45.0f), 0));
    Vector3 left45 = yaw45 * forward;
    Vector3 right45 = yawM45 * forward;

    Vector3 origin = myPos + Vector3(0, 0.5f, 0) + forward * 0.25f; // origin now forward & elevated

    unsigned short maskBits = 0;
    for (auto& name : m_layerNames)
    {
        maskBits |= static_cast<unsigned short>(LayerManager::GetInstance().GetLayer(name));
    }

    auto& phys = Physics::GetInstance();
    RaycastHit hitC = phys.Raycast(origin, forward, m_wallDetectDistance, maskBits);
    RaycastHit hitL = phys.Raycast(origin, left45, m_wallDetectDistance, maskBits);
    RaycastHit hitR = phys.Raycast(origin, right45, m_wallDetectDistance, maskBits);

    Vector3 moveDir = dir;
    if (hitC.hasHit)
    {
        if (!hitL.hasHit)      moveDir = left45;
        else if (!hitR.hasHit) moveDir = right45;
        else                   moveDir = -forward; // blocked all sides
    }
    else if (hitL.hasHit && !hitR.hasHit) moveDir = right45;
    else if (hitR.hasHit && !hitL.hasHit) moveDir = left45;

    float avoidWeight = 1.5f;
    float t = avoidWeight / (1.0f + avoidWeight);
    Vector3 blended = Vector3::Lerp(dir, moveDir, t);
    blended.Normalize();

    // Set velocity
    glm::vec3 vel = m_rigidbody->GetLinearVelocity();
    vel.x = blended.x * m_speed;
    vel.z = blended.z * m_speed;
    m_rigidbody->SetLinearVelocity(vel);

    // Smooth rotation toward movement direction
    if (blended.Length() > 0.01f)
    {
        float targetYaw = std::atan2(blended.x, blended.z); // radians
        Quaternion targetRot = Quaternion::FromEuler(Vector3(0.0f, targetYaw, 0.0f)); // radians

        float rotationSpeed = 10.0f;
        transform.localRotation = Quaternion::Slerp(transform.localRotation, targetRot, Time::FixedDeltaTime * rotationSpeed);
    }
}

void Enemy::TakeDamage(int damage)
{
    m_health -= damage;
    if (m_health <= 0)
    {
        Destroy(m_owner);
    }
}

