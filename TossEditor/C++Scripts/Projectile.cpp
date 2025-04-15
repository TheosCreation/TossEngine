#include "Projectile.h"

void Projectile::OnInspectorGUI()
{
    Component::OnInspectorGUI();

    FloatSliderField("Projectile Speed", m_projectileSpeed);
}

json Projectile::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this));
    data["projectileSpeed"] = m_projectileSpeed;

    return data;
}

void Projectile::deserialize(const json& data)
{
    if (data.contains("projectileSpeed"))
    {
        m_projectileSpeed = data["projectileSpeed"].get<float>();
    }
}

void Projectile::onStart()
{
    m_rigidBody = m_owner->getComponent<Rigidbody>();
    Transform& transform = getTransform();
    m_rigidBody->AddForce(transform.GetForward() * m_projectileSpeed);
}
