#include "PlayerController.h"
#include "AudioEngine.h"
#include "Camera.h"
#include "Rigidbody.h"
#include "Collider.h"
#include "GroundCheck.h"

void PlayerController::OnInspectorGUI()
{
    Component::OnInspectorGUI();

    FloatSliderField("Movement Speed", m_movementSpeed, 0.1f);
    FloatSliderField("Acceleration", m_acceleration, 0.1f);
    FloatSliderField("Air Acceleration", m_airAcceleration, 0.1f);
    FloatSliderField("Jump Force", m_jumpForce, 1.0f);
    FloatSliderField("Jump Cooldown", m_jumpCooldown, 0.1f, 0.1f, 2.0f);

    vector<std::string> layers = m_layerNames;
    if (LayerDropdownField("Raycast hit layers", layers))
    {
        m_layerNames = layers;
    }
}

void PlayerController::onCreate()
{
    m_layerNames.push_back("Default");

    auto& resourceManager = ResourceManager::GetInstance();
    fireSound = resourceManager.createSound(SoundDesc(), "Fire", "Resources/Audio/fire.ogg");
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
        m_rigidBody->AddForce(Vector3(0.0f, 1.0f, 0.0f) * m_jumpForce);
        m_groundCheck->isGrounded = false;
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
        Vector3 desiredVelocity = Vector3(moveDirection.x * m_movementSpeed, velocity.y, moveDirection.z * m_movementSpeed);
        Vector3 velocityChange = desiredVelocity - velocity;

        // If on the ground, set the linear velocity directly
        if (m_groundCheck->isGrounded)
        {
            // Limit acceleration
            Vector3 accelerationStep = velocityChange.Normalized() * m_acceleration * Time::FixedDeltaTime;

            // Don't overshoot
            if (accelerationStep.Length() > velocityChange.Length())
                accelerationStep = velocityChange;

            m_rigidBody->SetLinearVelocity(velocity + accelerationStep);
        }
        //else
        //{
        //    // Limit acceleration
        //    Vector3 accelerationStep = velocityChange.Normalized() * m_airAcceleration * Time::FixedDeltaTime;
        //
        //    // Don't overshoot
        //    if (accelerationStep.Length() > velocityChange.Length())
        //        accelerationStep = velocityChange;
        //
        //    // If not on the ground, apply a force for movement
        //    Vector3 force = accelerationStep;
        //    m_rigidBody->AddForce(force);
        //}
    }
}

json PlayerController::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this)); // Store the component type
    data["layerNames"] = m_layerNames;
    data["movementSpeed"] = m_movementSpeed;
    data["acceleration"] = m_acceleration;
    data["airAcceleration"] = m_airAcceleration;
    data["jumpForce"] = m_jumpForce;
    data["jumpCooldown"] = m_jumpCooldown;

    return data;
}

void PlayerController::deserialize(const json& data)
{
    if (data.contains("layerNames")) {
        m_layerNames = data["layerNames"].get<std::vector<std::string>>();
    }

    if (data.contains("movementSpeed")) {
        m_movementSpeed = data["movementSpeed"].get<float>();
    }

    if (data.contains("acceleration")) {
        m_acceleration = data["acceleration"].get<float>();
    }

    if (data.contains("airAcceleration")) {
        m_airAcceleration = data["airAcceleration"].get<float>();
    }

    if (data.contains("jumpForce")) {
        m_jumpForce = data["jumpForce"].get<float>();
    }

    if (data.contains("jumpCooldown")) {
        m_jumpCooldown = data["jumpCooldown"].get<float>();
    }
}