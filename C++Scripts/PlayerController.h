#pragma once
#include "TossEngine.h"

class PlayerController : public Component
{
public:
    PlayerController() = default;
    ~PlayerController() = default;

    virtual void onCreate() override;
    virtual void onUpdate(float deltaTime) override;
private:
    float m_elapsedSeconds = 0.0f; // Elapsed time in seconds

    float m_minSpeed = 1.0f; // 
    float m_maxSpeed = 500.0f; // 
    float m_movementSpeed = 50.0f; // Movement speed of the movable object
    float m_speedAdjustmentFactor = 2.0f; // 
    float m_yaw = 0.0f; // Yaw angle for rotation of the camera
    float m_pitch = 0.0f; // Pitch angle for rotation of the camera
    float m_minFov = 1.0f; // Minimum fov for the camera
    float m_fov = 90.0f; // Fov for the camera
    float m_maxFov = 120.0f; // Maximum fov for the camera
    float m_zoomSpeed = 0.5f; // Speed of zooming
    bool m_playMode = true; // Flag for locking the cursor
    bool m_wireframeMode = false; // Flag for wireframe mode

    Camera* m_cam = nullptr; // Pointer to the main camera
    SoundPtr fireSound = nullptr;
};

REGISTER_COMPONENT(PlayerController);