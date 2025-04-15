#pragma once
#include <TossEngine.h>

class Camera;
class Rigidbody;
class Collider;
class GroundCheck;

class PlayerController : public Component
{
public:
    void OnInspectorGUI() override;

    void onCreate() override;
    void onStart() override;
    void onUpdate() override;
    void onFixedUpdate() override;
    json serialize() const override;
    void deserialize(const json& data) override;

private:
    int m_health = 100;
    float m_movementSpeed = 35.0f; // Movement speed of the movable object
    float m_acceleration = 200.0f;
    float m_airAcceleration = 10000.0f;
    float m_jumpForce = 2000.0f;
    float m_jumpCooldown = 0.5f;

    float jumpTimer = 0.0f;

    float m_yaw = 0.0f; // Yaw angle for rotation of the camera
    float m_pitch = 0.0f; // Pitch angle for rotation of the camera
    bool m_wireframeMode = false; // Flag for wireframe mode

    vector<std::string> m_layerNames;

    Camera* m_cam = nullptr; // Pointer to the main camera
    Rigidbody* m_rigidBody = nullptr; // Pointer to the rigidbody
    GroundCheck* m_groundCheck = nullptr; // Pointer to the groundCheck
    SoundPtr fireSound = nullptr;
};

REGISTER_COMPONENT(PlayerController);