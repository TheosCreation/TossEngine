#include "PlayerController.h"
#include "AudioEngine.h"
#include "Camera.h"
#include "Rigidbody.h"
#include "Collider.h"

void PlayerController::OnInspectorGUI()
{
    ImGui::Text("Player Controller Inspector - ID: %p", this);
    ImGui::Separator();

    ImGui::DragFloat("Movement Speed", &m_movementSpeed, 0.1f);
    ImGui::DragFloat("Acceleration", &m_acceleration, 0.1f);
    ImGui::DragFloat("Air Acceleration", &m_airAcceleration, 0.1f);
    ImGui::DragFloat("Jump Force", &m_jumpForce, 1.0f);
    ImGui::DragFloat("Jump Cooldown", &m_jumpForce, 0.1f, 0.1f, 2.0f);
}

void PlayerController::onCreate()
{

    auto& resourceManager = ResourceManager::GetInstance();
    fireSound = resourceManager.createSound(SoundDesc(), "Fire", "Resources/Audio/fire.ogg");
    auto& inputManager = InputManager::GetInstance();
    inputManager.enablePlayMode(true);
}

void PlayerController::onStart()
{
    m_rigidBody = m_owner->getComponent<Rigidbody>();
    m_cam = m_owner->getComponentInChildren<Camera>();
}

void PlayerController::onUpdate(float deltaTime)
{
    auto& inputManager = InputManager::GetInstance();
    auto& audioEngine = AudioEngine::GetInstance();
    auto& tossEngine = TossEngine::GetInstance();
    if (tossEngine.IsDebugMode() && inputManager.isKeyPressed(KeyF1))
    {
        inputManager.togglePlayMode();
    }

    if (!inputManager.isPlayModeEnabled()) return;

    if (inputManager.isKeyPressed(Key::Key1))
    {
        audioEngine.playSound(fireSound);
    }

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
    m_owner->m_transform.localRotation = yawRotation;
    m_cam->getOwner()->m_transform.localRotation = pitchRotation;

    // Toggle wireframe mode on/off
    if (inputManager.isKeyPressed(Key::KeyR))
    {
        RaycastHit hit = Physics::GetInstance().Raycast(m_cam->getOwner()->m_transform.position, m_cam->getOwner()->m_transform.GetForward());
        if (hit.hasHit)
        {
            Debug::Log(hit.collider->getOwner()->name);
        }
    }
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

    if (inputManager.isKeyPressed(Key::KeySpace) && jumpTimer <= 0 && m_onGround)
    {
        m_rigidBody->AddForce(Vector3(0.0f, 1.0f, 0.0f) * m_jumpForce);
        jumpTimer = m_jumpCooldown;
        m_onGround = false;
    }
    else
    {
        jumpTimer -= deltaTime;
    }


    Vector3 forward = m_owner->m_transform.GetForward();
    Vector3 right = m_owner->m_transform.GetRight();
    Vector3 velocity = m_rigidBody->GetLinearVelocity();

    Vector3 moveDirection(0.0f);

    // Determine the move direction based on input
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

        // If on the ground, set the linear velocity directly
        if (m_onGround)
        {
            // Limit acceleration
            Vector3 accelerationStep = velocityChange.Normalized() * m_acceleration * deltaTime;

            // Don't overshoot
            if (accelerationStep.Length() > velocityChange.Length())
                accelerationStep = velocityChange;

            m_rigidBody->SetLinearVelocity(velocity + accelerationStep);
        }
        else
        {
            // Limit acceleration
            Vector3 accelerationStep = velocityChange.Normalized() * m_airAcceleration * deltaTime;

            // Don't overshoot
            if (accelerationStep.Length() > velocityChange.Length())
                accelerationStep = velocityChange;

            // If not on the ground, apply a force for movement
            Vector3 force = accelerationStep;
            m_rigidBody->AddForce(force);
        }
    }
}

void PlayerController::onCollisionEnter(Collider* other)
{
    if (other->getOwner()->tag == "Ground")
    {
        m_onGround = true;
    }
}

void PlayerController::onCollisionExit(Collider* other)
{
    if (other->getOwner()->tag == "Ground")
    {
        m_onGround = false;
    }
}
