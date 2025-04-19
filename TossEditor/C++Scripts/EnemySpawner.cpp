#include "EnemySpawner.h"

void EnemySpawner::OnInspectorGUI()
{
    Component::OnInspectorGUI();

    FloatSliderField("Spawn Interval", m_spawnInterval);
    ResourceAssignableField(m_enemyPrefab, "Enemy Prefab");
}

void EnemySpawner::onUpdate()
{
    spawnTimer -= Time::DeltaTime;
    if (spawnTimer <= 0.0f)
    {
        spawnTimer = m_spawnInterval;
        m_owner->getGameObjectManager()->Instantiate(m_enemyPrefab);
    }
}
