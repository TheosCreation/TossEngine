#pragma once
#include <TossEngine.h>

class GroundCheck : public Component
{
public:
    void OnInspectorGUI() override;
    void onTriggerEnter(Collider* other) override;
    void onTriggerExit(Collider* other) override;

    bool isGrounded = false;
};

REGISTER_COMPONENT(GroundCheck);