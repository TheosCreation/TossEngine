#include "Enemy.h"

void Enemy::OnInspectorGUI()
{
    IntSliderField("Health", m_health);

    ImGui::DragFloat3("Position", vector.Data(), 0.1f);
}

void Enemy::TakeDamage(int damage)
{
    m_health -= damage;
    if (m_health <= 0)
    {
        Destroy(m_owner);
    }
}

