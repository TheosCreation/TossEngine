#include "Enemy.h"

void Enemy::OnInspectorGUI()
{
    IntSliderField("Health", m_health);
}

void Enemy::TakeDamage(int damage)
{
    m_health -= damage;
}
