#pragma once
#include "GameObject.h"
class Collider;
class Rigidbody;
class TossEditor;

class EditorPickProxy final : public GameObject {
public:
    explicit EditorPickProxy(TossEditor* editor, std::weak_ptr<GameObject> target)
        : m_target(std::move(target)), m_editor(editor) {

        if (auto spt = m_target.lock()) m_targetId = spt->getId();

        name = "EditorProxy_" + std::to_string(m_targetId);
        isActive = true;
    }

    void onCreate() override;
    void onUpdateInternal() override;
    bool Delete(bool) override; // delete proxy only

    /**
     * @brief Gets the Physics World associated with this EditorPickProxy.
     * @return Pointer to the physics world (may be nullptr if EditorPickProxy is not simulated).
     */
    reactphysics3d::PhysicsWorld* getWorld() override;

    std::weak_ptr<GameObject> Target() const { return m_target; }
    size_t TargetId() const { return m_targetId; }

private:
    std::weak_ptr<GameObject> m_target;
    size_t m_targetId = 0;
    Collider* m_collider = nullptr;
    Rigidbody* m_rigidbody = nullptr;
    TossEditor* m_editor = nullptr;
};
