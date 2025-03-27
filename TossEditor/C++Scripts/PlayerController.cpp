#include "PlayerController.h"
#include "AudioEngine.h"
#include "Camera.h"

void PlayerController::onCreate()
{
    m_cam = m_owner->addComponent<Camera>();

    auto& resourceManager = ResourceManager::GetInstance();
    fireSound = resourceManager.createSound(SoundDesc(), "Fire", "Resources/Audio/fire.ogg");
    auto& inputManager = InputManager::GetInstance();
    inputManager.enablePlayMode(m_playMode);
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

    Vector2 mouseScroll = inputManager.getMouseScroll();
    if (inputManager.isKeyDown(Key::KeyLeftControl))
    {
        if (mouseScroll.y < 0 || mouseScroll.y > 0)
        {
            //adjust the movement speed
            m_movementSpeed += mouseScroll.y * m_speedAdjustmentFactor;
            m_movementSpeed = Clamp(m_movementSpeed, m_minSpeed, m_maxSpeed);
        }
    }
    else
    {
        if (mouseScroll.y < 0 || mouseScroll.y > 0)
        {
            m_fov -= mouseScroll.y * m_zoomSpeed;
            m_fov = Clamp(m_fov, m_minFov, m_maxFov);
            m_cam->setFieldOfView(m_fov);
        }
    }

    float adjustedMoveSpeed = m_movementSpeed;
    // Adjust camera speed if Shift key is pressed
    if (inputManager.isKeyDown(Key::KeyShift)) {
        adjustedMoveSpeed = m_movementSpeed * 2.0f;
    }


    Vector3 forward = m_owner->m_transform.GetForward();
    Vector3 right = m_owner->m_transform.GetRight();
    Vector3 up = m_owner->m_transform.GetUp();

    if (inputManager.isKeyDown(Key::KeyW))
        m_owner->m_transform.position += forward * adjustedMoveSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyS))
        m_owner->m_transform.position -= forward * adjustedMoveSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyA))
        m_owner->m_transform.position -= right * adjustedMoveSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyD))
        m_owner->m_transform.position += right * adjustedMoveSpeed * deltaTime;

    // Handle input for player rotation
    if (inputManager.isKeyDown(Key::KeyQ))
        m_owner->m_transform.position -= up * m_movementSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyE))
        m_owner->m_transform.position += up * m_movementSpeed * deltaTime;
}