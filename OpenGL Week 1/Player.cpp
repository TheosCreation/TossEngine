/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : MyPlayer.cpp
Description : MyPlayer class is an entity that can be adjusted by the end user
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "Player.h"
#include <algorithm>

Player::Player()
{
}

Player::~Player()
{
}

void Player::onCreate()
{
    m_cam = std::make_unique<Camera>();
    addComponent<Camera>(m_cam.get());

    //m_uiCamera = std::make_unique<Camera>();
    //addComponent<Camera>(m_uiCamera.get());
    //m_uiCamera->setCameraType(CameraType::Orthogonal);
}

void Player::onUpdate(float deltaTime)
{
    auto& inputManager = InputManager::GetInstance();

    if (!inputManager.isMouseDown(MouseButtonLeft))
    {
        m_playMode = false;
        inputManager.enablePlayMode(m_playMode);
        return;
    }

    m_playMode = true;
    inputManager.enablePlayMode(m_playMode);


    auto& lightManager = LightManager::GetInstance();

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
    m_transform.rotation = yawRotation * pitchRotation;

    // Enable play mode when clicking on the window
    //if (inputManager.isMousePressed(MouseButtonLeft) && !m_playMode)
    //{
    //    m_playMode = true;
    //    inputManager.enablePlayMode(m_playMode);
    //}
    //
    //// Disable play mode when pressing the Escape key
    //if (inputManager.isKeyPressed(Key::KeyEscape) && m_playMode)
    //{
    //    m_playMode = false;
    //    inputManager.enablePlayMode(m_playMode);
    //}

    //// Toggle point lights on/off
    //if (inputManager.isKeyPressed(Key::Key1))
    //{
    //    bool currentLightStatus = lightManager.getPointLightsStatus();
    //    lightManager.setPointLightsStatus(!currentLightStatus);
    //}


    //// Toggle directional light on/off
    //if (inputManager.isKeyPressed(Key::Key2))
    //{
    //    bool currentLightStatus = lightManager.getDirectionalLightStatus();
    //    lightManager.setDirectionalLightStatus(!currentLightStatus);
    //}

    //// Toggle spotlight on/off
    //if (inputManager.isKeyPressed(Key::Key3))
    //{
    //    bool currentLightStatus = lightManager.getSpotlightStatus();
    //    lightManager.setSpotlightStatus(!currentLightStatus);
    //}

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

    //// Get the mouse position from the input manager
    //glm::vec2 cursorPosition = inputManager.getMousePosition();

    //// Toggle to print the cords of the cursor
    //if (inputManager.isKeyPressed(Key::Key5))
    //{
    //    std::cout << "Mouse Coordinates: (" << cursorPosition.x << ", " << cursorPosition.y << ")" << std::endl;
    //}
    // Adjust camera speed if Shift key is pressed
    if (inputManager.isKeyDown(Key::KeyShift)) {
        m_movementSpeed = m_originalMovementSpeed * 2.0f;
    }
    else {
        m_movementSpeed = m_originalMovementSpeed;
    }

    Vector3 forward = m_transform.GetForward();
    Vector3 right = m_transform.GetRight();
    Vector3 up = m_transform.GetUp();
    
    if (inputManager.isKeyDown(Key::KeyW))
        m_transform.position += forward * m_movementSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyS))
        m_transform.position -= forward * m_movementSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyA))
        m_transform.position -= right * m_movementSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyD))
        m_transform.position += right * m_movementSpeed * deltaTime;

    // Handle input for player rotation
    if (inputManager.isKeyDown(Key::KeyQ))
        m_transform.position -= up * m_movementSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyE))
        m_transform.position += up * m_movementSpeed * deltaTime; 
    
    Vector2 mouseScroll = inputManager.getMouseScroll();
    if (mouseScroll.y < 0 || mouseScroll.y > 0)
    {
        m_fov -= mouseScroll.y * m_zoomSpeed;

        // Clamp the FOV to prevent it from going out of a reasonable range
        if (m_fov < m_minFov) {
            m_fov = m_minFov;
        }
        if (m_fov > m_maxFov) {
            m_fov = m_maxFov;
        }
        m_cam->setFieldOfView(m_fov);
    }
}

void Player::onFixedUpdate(float fixedDeltaTime)
{
    // do physics here
}

void Player::onLateUpdate(float deltaTime)
{
}
