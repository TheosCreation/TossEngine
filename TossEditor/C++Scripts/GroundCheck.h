#pragma once
#include <TossEngine.h>

class GroundCheck : public Component
{
public:
    void onTriggerEnter(Collider* other) override;
    
};

REGISTER_COMPONENT(GroundCheck);