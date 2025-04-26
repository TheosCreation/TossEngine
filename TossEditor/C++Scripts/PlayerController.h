#pragma once
#include <TossEngine.h>

class GroundCheck;

class PlayerController : public Component
{
public:
    void OnInspectorGUI() override;

    void onCreate() override;
    void onStart() override;
    void onLateStart() override;
    void onUpdate() override;
    void onFixedUpdate() override;

private:
    int m_health = 100;
    float m_movementSpeed = 35.0f; // Movement speed of the movable object
    float m_acceleration = 200.0f;
    float m_airAcceleration = 10000.0f;
    float m_jumpForce = 2000.0f;
    float m_jumpCooldown = 0.5f;
    float m_currentLevelTime = 60.0f;

    float jumpTimer = 0.0f;

    float m_yaw = 0.0f; // Yaw angle for rotation of the camera
    float m_pitch = 0.0f; // Pitch angle for rotation of the camera
    bool m_wireframeMode = false; // Flag for wireframe mode

    vector<std::string> m_layerNames;

    Camera* m_cam = nullptr; // Pointer to the main camera
    Rigidbody* m_rigidBody = nullptr; // Pointer to the rigidbody
    GroundCheck* m_groundCheck = nullptr; // Pointer to the groundCheck
    Texture2DPtr m_texture = nullptr;
    TextureCubeMapPtr m_cubeMapTexture = nullptr;

    SERIALIZABLE_MEMBERS(m_health, m_movementSpeed, m_acceleration, m_airAcceleration, m_jumpForce, m_jumpCooldown, m_layerNames, m_texture, m_cubeMapTexture, m_currentLevelTime)
};

REGISTER_COMPONENT(PlayerController);