#include "Enemy.h"

void Enemy::OnInspectorGUI()
{
    IntSliderField("Health", m_health);
}

void Enemy::TakeDamage(int damage)
{
    m_health -= damage;
    if (m_health <= 0)
    {
        Destroy(m_owner);
    }
}
