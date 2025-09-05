#include "EditorPickProxy.h"

#include "Collider.h"
#include "Rigidbody.h"
#include "TossEditor.h"

void EditorPickProxy::onCreate()
{
    // No visuals. Collider + static rigidbody only.
    m_collider = addComponent<Collider>();
    //m_collider->SetTrigger(true);                   // we only need ray hits

    m_rigidbody = addComponent<Rigidbody>();
    m_rigidbody->SetBodyType(BodyType::STATIC);

    if (auto tgt = m_target.lock().get()) {
        // Copy transform and create box collider
        m_transform.SetMatrix(tgt->m_transform.GetMatrix());
        m_collider->SetBoxCollider(Vector3(1.0f));
    }
}

void EditorPickProxy::onUpdateInternal()
{
    GameObject::onUpdateInternal();
    m_rigidbody->SetIsDebugEnabled(true);
    if (auto tgt = m_target.lock()) {
        // Keep transform matched every frame
        m_transform.SetMatrix(tgt->m_transform.GetMatrix());
    }
    else {
        Delete(true);
    }
}

bool EditorPickProxy::Delete(bool cond)
{
    if (isDestroyed) return false;

    if (m_transform.parent)
        m_transform.SetParent(nullptr);

    // Remove this game object from the scene.
    m_editor->DeleteProxy(m_targetId);
    return true;
}

reactphysics3d::PhysicsWorld* EditorPickProxy::getWorld()
{
    return Physics::GetInstance().GetEditorWorld();
}