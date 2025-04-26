#include "PlayerController.h"
#include "AudioEngine.h"
#include "GroundCheck.h"
#include "UiManager.h"

void PlayerController::OnInspectorGUI()
{
    Component::OnInspectorGUI();

    IntSliderField("Health", m_health);
    FloatSliderField("Movement Speed", m_movementSpeed, 0.1f);
    FloatSliderField("Acceleration", m_acceleration, 0.1f);
    FloatSliderField("Air Acceleration", m_airAcceleration, 0.1f);
    FloatSliderField("Jump Force", m_jumpForce, 1.0f);
    FloatSliderField("Jump Cooldown", m_jumpCooldown, 0.1f, 0.1f, 2.0f);
    ResourceAssignableField(m_texture, "Texture");

    vector<std::string> layers = m_layerNames;
    if (LayerDropdownField("Raycast hit layers", layers))
    {
        m_layerNames = layers;
    }
}

void PlayerController::onCreate()
{
    m_layerNames.push_back("Default");
}

void PlayerController::onStart()
{
    m_rigidBody = m_owner->getComponent<Rigidbody>();
    m_groundCheck = m_owner->getComponentInChildren<GroundCheck>();
    m_cam = m_owner->getComponentInChildren<Camera>();
    auto& inputManager = InputManager::GetInstance();
    inputManager.enablePlayMode(true);
}

void PlayerController::onUpdate()
{
    auto& inputManager = InputManager::GetInstance();
    auto& audioEngine = AudioEngine::GetInstance();
    auto& tossEngine = TossEngine::GetInstance();
    if (tossEngine.IsDebugMode() && inputManager.isKeyPressed(KeyF1, false))
    {
        inputManager.togglePlayMode();
    }

    float sensitivity = 0.1f;  // Sensitivity factor for mouse movement
    m_yaw -= inputManager.getMouseXAxis() * sensitivity;
    m_pitch -= inputManager.getMouseYAxis() * sensitivity;
    //
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
        unsigned short maskBits = 0;
        for (const auto& layerName : m_layerNames) {
            maskBits |= static_cast<unsigned short>(LayerManager::GetInstance().GetLayer(layerName));
        }

        RaycastHit hit = Physics::GetInstance().Raycast(m_cam->getOwner()->m_transform.position, m_cam->getOwner()->m_transform.GetForward(), 1000.0f, maskBits);
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

    if (inputManager.isKeyPressed(Key::KeySpace) && jumpTimer <= 0 && m_groundCheck->isGrounded)
    {
        m_rigidBody->AddForce(Vector3::Up * m_jumpForce);
        jumpTimer = m_jumpCooldown;
    }
    else
    {
        jumpTimer -= Time::DeltaTime;
    }
}

void PlayerController::onFixedUpdate()
{


    auto& inputManager = InputManager::GetInstance();
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
        velocityChange.y = 0.0f;

        // If on the ground, set the linear velocity directly
        if (m_groundCheck->isGrounded)
        {
            // Limit acceleration
            Vector3 accelerationStep = velocityChange.Normalized() * m_acceleration * Time::FixedDeltaTime;

            // Don't overshoot
            if (accelerationStep.Length() > velocityChange.Length())
                accelerationStep = velocityChange;

            if (accelerationStep.Length() > 0)
                m_rigidBody->SetLinearVelocity(velocity + accelerationStep);
        }
        else
        {
            Vector3 desiredMaxVelocity = Vector3(moveDirection.x * m_movementSpeed, 0, moveDirection.z * m_movementSpeed);
            if ((desiredMaxVelocity.x > 0.0f && velocity.x > m_movementSpeed) || (desiredMaxVelocity.x < 0.0f && velocity.x < -m_movementSpeed))
            {
                desiredMaxVelocity.x = 0;
            }
            if ((desiredMaxVelocity.z > 0.0f && velocity.z > m_movementSpeed) || (desiredMaxVelocity.z < 0.0f && velocity.z < -m_movementSpeed))
            {
                desiredMaxVelocity.z = 0;
            }
            // Limit acceleration
            Vector3 force = desiredMaxVelocity * m_airAcceleration * Time::FixedDeltaTime;
            m_rigidBody->AddForce(force);
        }
    }
}