#include "EnemySpawner.h"

void EnemySpawner::OnInspectorGUI()
{
    Component::OnInspectorGUI();

    FloatSliderField("Spawn Interval", m_spawnInterval);
    ResourceAssignableField(m_enemyPrefab, "Enemy Prefab");
    VectorField(m_spawnpointObjects, "Spawn Points");
}

void EnemySpawner::onUpdate()
{
    spawnTimer -= Time::DeltaTime;
    if (spawnTimer <= 0.0f)
    {
        spawnTimer = m_spawnInterval;

        if (!m_enemyPrefab) return;
        if (m_spawnpointObjects.empty()) return;

        int randomIndex = Random::Range(0, static_cast<int>(m_spawnpointObjects.size()) - 1);
        GameObjectPtr spawnPoint = m_spawnpointObjects[randomIndex];

        if (!spawnPoint) return;

        m_owner->getGameObjectManager()->Instantiate(m_enemyPrefab, spawnPoint->m_transform.position, spawnPoint->m_transform.rotation);
    }
}
