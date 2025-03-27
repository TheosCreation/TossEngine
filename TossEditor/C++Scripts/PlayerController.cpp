#include "PlayerController.h"
#include "AudioEngine.h"
#include "Camera.h"
#include "Rigidbody.h"

void PlayerController::onCreate()
{
    m_cam = m_owner->addComponent<Camera>();

    auto& resourceManager = ResourceManager::GetInstance();
    fireSound = resourceManager.createSound(SoundDesc(), "Fire", "Resources/Audio/fire.ogg");
    auto& inputManager = InputManager::GetInstance();
    inputManager.enablePlayMode(m_playMode);
}

void PlayerController::onStart()
{
    m_rigidBody = m_owner->getComponent<Rigidbody>();
}

void PlayerController::onUpdate(float deltaTime)
{
    auto& inputManager = InputManager::GetInstance();
    auto& audioEngine = AudioEngine::GetInstance();
    auto& tossEngine = TossEngine::GetInstance();
    if (inputManager.isKeyPressed(Key::Key1))
    {
        audioEngine.playSound(fireSound);
    }
    if (tossEngine.IsDebugMode() && inputManager.isKeyPressed(KeyF1))
    {
        m_playMode = !m_playMode;
        inputManager.enablePlayMode(m_playMode);
    }

    if (!m_playMode) return;

    float sensitivity = 0.1f;  // Sensitivity factor for mouse movement
    m_yaw -= inputManager.getMouseXAxis() * sensitivity;
    m_pitch -= inputManager.getMouseYAxis() * sensitivity;

    // Clamp the pitch value to prevent flipping the camera
    if (m_pitch > 89.0f)
        m_pitch = 89.0f;
    if (m_pitch < -89.0f)
        m_pitch = -89.0f;

    // Create quaternions for yaw (around the y-axis) and pitch (around the x-axis)
    glm::quat yawRotation = glm::angleAxis(glm::radians(m_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat pitchRotation = glm::angleAxis(glm::radians(m_pitch), glm::vec3(1.0f, 0.0f, 0.0f));

    // Combine yaw and pitch rotations and apply to player transform
    m_owner->m_transform.rotation = yawRotation * pitchRotation;

    // Toggle wireframe mode on/off
    if (inputManager.isKeyPressed(Key::Key0))
    {
        m_wireframeMode = !m_wireframeMode;
        if (m_wireframeMode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    if (inputManager.isKeyPressed(Key::KeySpace) && jumpTimer <= 0)
    {
        m_rigidBody->AddForce(Vector3(0.0f, 1.0f, 0.0f) * m_jumpForce);
        jumpTimer = m_jumpCooldown;
    }
    else
    {
        jumpTimer -= deltaTime;
    }


    Vector3 forward = m_owner->m_transform.GetForward();
    Vector3 right = m_owner->m_transform.GetRight();
    Vector3 velocity = m_rigidBody->GetLinearVelocity();

    Vector3 moveDirection(0.0f);

    if (inputManager.isKeyDown(Key::KeyW)) moveDirection += forward;
    if (inputManager.isKeyDown(Key::KeyS)) moveDirection -= forward;
    if (inputManager.isKeyDown(Key::KeyA)) moveDirection -= right;
    if (inputManager.isKeyDown(Key::KeyD)) moveDirection += right;

    if (moveDirection.Length() > 0.0f)
    {
        moveDirection = moveDirection.Normalized();

        // Accelerate toward direction
        Vector3 desiredVelocity = moveDirection * m_movementSpeed;
        Vector3 velocityChange = desiredVelocity - velocity;

        // Limit acceleration
        Vector3 accelerationStep = velocityChange.Normalized() * m_acceleration * deltaTime;

        // Don't overshoot
        if (accelerationStep.Length() > velocityChange.Length())
            accelerationStep = velocityChange;

        m_rigidBody->SetLinearVelocity(velocity + accelerationStep);
    }
    else
    {
        // Dampen velocity when no input
        //velocity *= 0.9f;
        m_rigidBody->SetLinearVelocity(velocity);
    }

}