#pragma once
#include <TossEngine.h>
#include <All.h>

class Camera;
class Rigidbody;

class PlayerController : public Component
{
public:
    PlayerController() = default;
    ~PlayerController() = default;

    virtual void onCreate() override;
    virtual void onStart() override;
    virtual void onUpdate(float deltaTime) override;
private:
    float m_elapsedSeconds = 0.0f; // Elapsed time in seconds

    float m_movementSpeed = 35.0f; // Movement speed of the movable object
    float m_acceleration = 50.0f;
    float m_jumpForce = 2000.0f;
    float m_jumpCooldown = 0.5f;

    float jumpTimer = 0.0f;

    float m_yaw = 0.0f; // Yaw angle for rotation of the camera
    float m_pitch = 0.0f; // Pitch angle for rotation of the camera
    float m_minFov = 1.0f; // Minimum fov for the camera
    float m_fov = 90.0f; // Fov for the camera
    float m_maxFov = 120.0f; // Maximum fov for the camera
    float m_zoomSpeed = 0.5f; // Speed of zooming
    bool m_playMode = true; // Flag for locking the cursor
    bool m_wireframeMode = false; // Flag for wireframe mode

    Camera* m_cam = nullptr; // Pointer to the main camera
    Rigidbody* m_rigidBody = nullptr; // Pointer to the rigidbody
    SoundPtr fireSound = nullptr;
};

REGISTER_COMPONENT(PlayerController);