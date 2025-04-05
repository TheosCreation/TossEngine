#pragma once
#include "Component.h"

class TOSSENGINE_API Ship : public Component
{
public:
    Ship() = default;
    ~Ship() = default;

    virtual void onUpdate() override;

private:
    float m_elapsedSeconds = 0.0f;
    float m_radius = 50.0f; // Radius of the circle
    float m_speed = 1.0f;   // Speed of the ship's movement
};

REGISTER_COMPONENT(Ship);