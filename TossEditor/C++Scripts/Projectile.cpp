#include "Projectile.h"
#include "Enemy.h"
#include "PlayerController.h"

void Projectile::OnInspectorGUI()
{
    Component::OnInspectorGUI();

    IntSliderField("Damage", m_damage);
    FloatSliderField("Projectile Speed", m_projectileSpeed);
}

void Projectile::onStart()
{
    m_rigidBody = m_owner->getComponent<Rigidbody>();
    Transform& transform = getTransform();
    m_rigidBody->AddForce(transform.GetForward() * m_projectileSpeed);
}

void Projectile::onTriggerEnter(Collider* other)
{
    GameObject* gameObject = other->getOwner();

    if (Enemy* enemy = gameObject->getComponent<Enemy>())
    {
        enemy->TakeDamage(m_damage);
        Destroy(m_owner);
        return;
    }

    if (PlayerController* player = gameObject->getComponent<PlayerController>())
    {
        Destroy(m_owner);
        return;
    }

    Destroy(m_owner);
}
