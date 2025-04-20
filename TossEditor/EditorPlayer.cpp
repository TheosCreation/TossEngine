#include "EditorPlayer.h"
#include "Camera.h"
#include "InputManager.h"
#include "TossEditor.h"
#include "TossEngine.h"
#include "Window.h"
//#include <imgui_internal.h>

#include <algorithm>

EditorPlayer::EditorPlayer(TossEditor* editor)
{
    Editor = editor;
}

EditorPlayer::~EditorPlayer()
{
}

void EditorPlayer::onCreate()
{
    auto& tossEngine = TossEngine::GetInstance();
    m_cam = addComponent<Camera>();
    m_cam->setScreenArea(tossEngine.GetWindow()->getInnerSize());
    tossEngine.SetDebugMode(true);
}

void EditorPlayer::Update(float deltaTime)
{
    auto& inputManager = InputManager::GetInstance(); 

    if (inputManager.isKeyDown(KeyLeftControl, false) && inputManager.isKeyPressed(KeyO, false))
    {
        Editor->OpenSceneViaFileSystem();
    }
    if (inputManager.isKeyDown(KeyLeftControl, false) && inputManager.isKeyPressed(KeyZ, false))
    {
        Editor->Undo();
    }
    if (inputManager.isKeyDown(KeyLeftControl, false) && inputManager.isKeyPressed(KeyS, false))
    {
        Editor->Save();
    }
    if (inputManager.isKeyDown(KeyLeftControl, false) && inputManager.isKeyPressed(KeyE, false))
    {
        Editor->Exit();
    }

    if (inputManager.isKeyDown(KeyLeftControl, false) && inputManager.isKeyPressed(KeyR, false))
    {
        Editor->Reload();
    }
    if (inputManager.isKeyDown(KeyLeftControl, false) && inputManager.isKeyPressed(KeyD, false))
    {
        Editor->DuplicateSelected();
    }
    if (inputManager.isKeyPressed(KeyDelete, false))
    {
        Editor->DeleteSelected();
    }

    if (inputManager.isMousePressed(MouseButtonLeft, false))
    {
        //Vector2 mousePosition = inputManager.getMousePosition();

        //Debug::Log(m_cam->screenToWorldPoint(mousePosition));
    }

    if (inputManager.isGameModeEnabled())
        return;


    if (!inputManager.isMouseDown(MouseButtonRight, false))
    {
        inputManager.enablePlayMode(false, false);
        return;
    }
    
    inputManager.enablePlayMode(true, false);

    float sensitivity = 0.1f;  // Sensitivity factor for mouse movement
    m_yaw -= inputManager.getMouseXAxis(false) * sensitivity;
    m_pitch -= inputManager.getMouseYAxis(false) * sensitivity;

    // Clamp the pitch value to prevent flipping the camera
    m_pitch = std::min(m_pitch, 89.0f);
    m_pitch = std::max(m_pitch, -89.0f);

    // Create quaternions for yaw (around the y-axis) and pitch (around the x-axis)
    glm::quat yawRotation = glm::angleAxis(glm::radians(m_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat pitchRotation = glm::angleAxis(glm::radians(m_pitch), glm::vec3(1.0f, 0.0f, 0.0f));

    // Combine yaw and pitch rotations and apply to player transform
    m_transform.localRotation = yawRotation * pitchRotation;

    Vector2 mouseScroll = inputManager.getMouseScroll();
    if (inputManager.isKeyDown(Key::KeyLeftControl, false))
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
    if (inputManager.isKeyDown(Key::KeyShift, false)) {
        adjustedMoveSpeed = m_movementSpeed * 2.0f;
    }


    Vector3 forward = m_transform.GetForward();
    Vector3 right = m_transform.GetRight();
    Vector3 up = m_transform.GetUp();

    //since the editor players tranform doesnt get updated to use local space we can manipulate position directly
    if (inputManager.isKeyDown(Key::KeyW, false))
        m_transform.localPosition += forward * adjustedMoveSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyS, false))
        m_transform.localPosition -= forward * adjustedMoveSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyA, false))
        m_transform.localPosition -= right * adjustedMoveSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyD, false))
        m_transform.localPosition += right * adjustedMoveSpeed * deltaTime;

    // Handle input for player rotation
    if (inputManager.isKeyDown(Key::KeyQ, false))
        m_transform.localPosition -= up * m_movementSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyE, false))
        m_transform.localPosition += up * m_movementSpeed * deltaTime;


    m_transform.UpdateWorldTransform();
}

Camera* EditorPlayer::getCamera()
{
    return m_cam;
}
