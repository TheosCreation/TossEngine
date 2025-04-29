#pragma once
#include <TossEngine.h>

class EnemySpawner : public Component
{
public:
    void OnInspectorGUI() override;
    void onUpdate() override;

private:
    PrefabPtr m_enemyPrefab = nullptr;
    float m_spawnInterval = 1.0f;

    vector<GameObjectPtr> m_spawnpointObjects;

    float spawnTimer = 0.0f;

    SERIALIZABLE_MEMBERS(m_enemyPrefab, m_spawnInterval, m_spawnpointObjects)
};

REGISTER_COMPONENT(EnemySpawner);