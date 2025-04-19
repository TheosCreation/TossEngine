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
}

void Enemy::onUpdate()
{
    if (!m_target || !m_rigidbody) return;

    // 1) get positions
    Vector3 myPos = getOwner()->m_transform.position;
    Vector3 playerPos = m_target->getOwner()->m_transform.position;

    // 2) compute horizontal delta only
    Vector3 delta = playerPos - myPos;
    delta.y = 0;                                 // ← ignore vertical
    float dist = delta.Length();
    if (dist < 0.1f) {                           // close enough
        // preserve any existing vertical (gravity) velocity:
        glm::vec3 v = m_rigidbody->GetLinearVelocity();
        v.x = v.z = 0;
        m_rigidbody->SetLinearVelocity(v);
        return;
    }

    // 3) horizontal direction
    Vector3 dir = delta / dist;

    // 4) build velocity: keep current Y so gravity still works
    Vector3 v = m_rigidbody->GetLinearVelocity();
    v.x = dir.x * m_speed;
    v.z = dir.z * m_speed;
    // v.y untouched

    m_rigidbody->SetLinearVelocity(v);
}


void Enemy::TakeDamage(int damage)
{
    m_health -= damage;
    if (m_health <= 0)
    {
        Destroy(m_owner);
    }
}

