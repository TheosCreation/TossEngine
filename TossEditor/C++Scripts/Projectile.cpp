#include "Projectile.h"
#include "Enemy.h"
#include "PlayerController.h"

void Projectile::OnInspectorGUI()
{
    Component::OnInspectorGUI();

    IntSliderField("Damage", m_damage);
    FloatSliderField("Projectile Speed", m_projectileSpeed);
}

json Projectile::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this));
    data["projectileSpeed"] = m_projectileSpeed;
    data["damage"] = m_damage;

    return data;
}

void Projectile::deserialize(const json& data)
{
    if (data.contains("projectileSpeed"))
    {
        m_projectileSpeed = data["projectileSpeed"].get<float>();
    }

    if (data.contains("damage"))
    {
        m_damage = data["damage"].get<int>();
    }
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

    if (gameObject->tag == "Projectile") return;
    if (gameObject->tag == "Ground")
    {
        Destroy(m_owner);
    }

    if (Enemy* enemy = gameObject->getComponent<Enemy>())
    {
        Debug::Log("Hit Enemy");
        enemy->TakeDamage(m_damage);
        Destroy(m_owner);
    }

    if (gameObject->getComponent<PlayerController>())
    {
        Debug::Log("Hit Player");
        Destroy(m_owner);
    }

    //hit something else
    //
}
