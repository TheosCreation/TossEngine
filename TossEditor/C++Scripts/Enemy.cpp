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

    // 1) Positions & horizontal delta
    Vector3 myPos = transform.position;
    Vector3 playerPos = m_target->getOwner()->m_transform.position;
    Vector3 delta = playerPos - myPos;
    delta.y = 0.0f;

    float dist = delta.Length();
    if (dist < 0.1f) {
        // close enough: zero XZ velocity, keep Y (gravity)
        glm::vec3 v = m_rigidbody->GetLinearVelocity();
        v.x = v.z = 0.0f;
        m_rigidbody->SetLinearVelocity(v);
        return;
    }

    // 2) Direction toward player (unit, horizontal)
    Vector3 dir = delta / dist;

    // 3) Build our “forward” quaternion + vector
    //    orient rotates (0,0,1) → dir
    Quaternion orient = Quaternion::LookAt(dir, Vector3::Up);
    Vector3    fw = orient * Vector3::Forward;

    // 4) Build ±45° yaw rays (around world‐up)
    Quaternion yaw45 = Quaternion::FromEuler(Vector3(0, 45, 0));
    Quaternion yawM45 = Quaternion::FromEuler(Vector3(0, -45, 0));
    Vector3   left45 = yaw45 * fw;
    Vector3   right45 = yawM45 * fw;

    // 5) Raycast against your wall layer mask
    Vector3 origin = myPos + Vector3(0, 0.5f, 0);
    unsigned short maskBits = 0;
    for (auto& name : m_layerNames) {
        maskBits |= static_cast<unsigned short>(
            LayerManager::GetInstance().GetLayer(name)
            );
    }
    auto& phys = Physics::GetInstance();
    RaycastHit hitC = phys.Raycast(origin, fw, m_wallDetectDistance, maskBits);
    RaycastHit hitL = phys.Raycast(origin, left45, m_wallDetectDistance, maskBits);
    RaycastHit hitR = phys.Raycast(origin, right45, m_wallDetectDistance, maskBits);

    // 6) Pick an avoidance direction
    Vector3 avoid = fw;
    if (hitC.hasHit) {
        if (!hitL.hasHit)        avoid = left45;
        else if (!hitR.hasHit)   avoid = right45;
        else                     avoid = -fw;      // all three blocked
    }
    else if (hitL.hasHit && !hitR.hasHit) avoid = right45;
    else if (hitR.hasHit && !hitL.hasHit) avoid = left45;

    // 7) Blend toward player vs avoid‐wall (using your Lerp + normalize)
    float avoidWeight = 1.5f;
    float t = avoidWeight / (1.0f + avoidWeight);
    Vector3 blended = Vector3::Lerp(dir, avoid, t);
    blended.Normalize();

    // 8) Apply velocity (preserve vertical)
    glm::vec3 vel = m_rigidbody->GetLinearVelocity();
    vel.x = blended.x * m_speed;
    vel.z = blended.z * m_speed;
    m_rigidbody->SetLinearVelocity(vel);

    // 9) Rotate the enemy to face its movement direction
    Quaternion lookRot = Quaternion::LookAt(blended, Vector3::Up);
    transform.localRotation = lookRot;
    transform.UpdateWorldTransform();
}

void Enemy::TakeDamage(int damage)
{
    m_health -= damage;
    if (m_health <= 0)
    {
        Destroy(m_owner);
    }
}

