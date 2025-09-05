#include "EditorPickProxy.h"

#include "Collider.h"
#include "Rigidbody.h"
#include "TossEditor.h"

void EditorPickProxy::onCreate()
{
    // No visuals. Collider + static rigidbody only.
    m_collider = addComponent<Collider>();
    m_collider->SetTrigger(true);                   // we only need ray hits

    m_rigidbody = addComponent<Rigidbody>();
    m_rigidbody->SetBodyType(BodyType::KINEMATIC);

    if (auto tgt = m_target.lock()) 
    {
        m_transform.SetMatrix(tgt->m_transform.GetMatrix());

        if (Collider* collider = tgt->getComponent<Collider>())
        {
            Vector3 extent = collider->GetExtent();
            m_collider->SetBoxCollider(extent);
        }
        else if (Renderer* renderer = tgt->getComponent<Image>())
        {
            m_collider->SetBoxCollider(renderer->GetExtent());
        }
        else
        {
            m_collider->SetBoxCollider(Vector3(1.0f));
        }
    }
}

void EditorPickProxy::onUpdateInternal()
{
    for (auto& pair : m_components) {
        if (pair.second)
        {
            pair.second->onUpdateInternal();
        }
    }

    m_rigidbody->onUpdate();
    m_rigidbody->SetIsDebugEnabled(true);
    if (auto tgt = m_target.lock()) {
        // Keep transform matched every frame
        m_transform.SetMatrix(tgt->m_transform.GetMatrix());
        m_transform.UpdateWorldTransform();
    }
    else {
        Delete(true);
    }
}

bool EditorPickProxy::Delete(bool) {
    isDestroyed = true;
    m_editor->MarkProxyForRemoval(m_targetId);
    return true;
}

reactphysics3d::PhysicsWorld* EditorPickProxy::getWorld()
{
    return Physics::GetInstance().GetEditorWorld();
}