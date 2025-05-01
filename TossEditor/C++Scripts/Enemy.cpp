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
    FloatSliderField("Attack Distance", m_attackDistance);
    IntSliderField("Attack Damage", m_attackDamage);
    FloatSliderField("Attack Cooldown", m_attackCoolDown);
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
    Vector3 dir = delta.Normalized();

    // Stop movement if within attack distance
    glm::vec3 vel = m_rigidbody->GetLinearVelocity();
    if (dist <= m_attackDistance)
    {
        vel.x = 0.0f;
        vel.z = 0.0f;
    }
    else
    {
        vel.x = dir.x * m_speed;
        vel.z = dir.z * m_speed;
    }
    m_rigidbody->SetLinearVelocity(vel);

    // Smooth rotation toward movement direction
    if (dir.Length() > 0.01f)
    {
        float targetYaw = std::atan2(dir.x, dir.z);
        Quaternion targetRot = Quaternion::FromEuler(Vector3(0.0f, targetYaw, 0.0f));

        float rotationSpeed = 10.0f;
        transform.localRotation = Quaternion::Slerp(transform.localRotation, targetRot, Time::FixedDeltaTime * rotationSpeed);
    }

    // Attack logic
    m_attackCooldownTimer -= Time::FixedDeltaTime;
    if (m_attackCooldownTimer <= 0.0f && dist <= m_attackDistance)
    {

        Vector3 rayOrigin = myPos + Vector3(0, 0.5f, 0);
        Vector3 rayTarget = playerPos + Vector3(0, 0.5f, 0);
        Vector3 rayDir = (rayTarget - rayOrigin).Normalized();

        RaycastHit playerHit = Physics::GetInstance().Raycast(rayOrigin, rayDir, m_attackDistance + 0.1f, ~0);

        if (playerHit.hasHit)
        {
            if (auto player = playerHit.collider->getOwner()->getComponent<PlayerController>())
            {
                player->TakeDamage(m_attackDamage);
            }
        }

        m_attackCooldownTimer = m_attackCoolDown;
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

