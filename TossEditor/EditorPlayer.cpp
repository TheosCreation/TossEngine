#include "EditorPlayer.h"
#include "Camera.h"
#include "InputManager.h"
#include "TossEditor.h"
#include "TossEngine.h"
#include "Window.h"
//#include <imgui_internal.h>

#include <algorithm>

#include "EditorPickProxy.h"

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
    m_cam->SetDrawUi(false);
}

void EditorPlayer::Update(float deltaTime)
{
    auto& inputManager = InputManager::GetInstance(); 
    auto& physics = Physics::GetInstance();

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
    if (inputManager.isKeyDown(KeyLeftControl, false) && inputManager.isKeyPressed(KeyN, false))
    {
        Editor->CreateScene();
    }
    //if (inputManager.isKeyDown(KeyLeftControl, false) && inputManager.isKeyPressed(KeyE, false))
    //{
    //    Editor->Exit();
    //}



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
    Vector2 mouse = InputManager::getMousePosition();
    Vector4 r = m_cam->getScreenArea();
    const bool insideSceneView =
        mouse.x >= r.x && mouse.y >= r.y &&
        mouse.x < r.x + r.z &&
        mouse.y < r.y + r.w;

    if (inputManager.isMousePressed(MouseButtonLeft, false) && insideSceneView && !ImGuizmo::IsOver() && !inputManager.isPlayModeEnabled()) {
        
        // build ray from mouse
        Vector3 nearW = m_cam->screenToWorldPoint(Vector3(mouse.x, mouse.y, m_cam->getNearPlane()));
        Vector3 farW = m_cam->screenToWorldPoint(Vector3(mouse.x, mouse.y, m_cam->getFarPlane()));
        Vector3 ro = nearW;
        Vector3 rd = (farW - nearW).Normalized();

        RaycastHit hit = physics.EditorRaycast(ro, rd, 10000.0f);

        if (hit.hasHit && hit.collider) {
            if (auto* proxy = dynamic_cast<EditorPickProxy*>(hit.collider->getOwner())) {
                if (auto target = proxy->Target().lock()) {
                    if (auto shared = target->getSharedOwner())
                        Editor->SetSelectedSelectable(shared);
                }
            }
        }
        else
        {
            Editor->SetSelectedSelectable(nullptr);
        }
    }
    if (inputManager.isGameModeEnabled())
        return;


    if (inputManager.isMouseDown(MouseButtonRight, false) && insideSceneView)
    {
        inputManager.enablePlayMode(true, false);
    }
    else
    {
        inputManager.enablePlayMode(false, false);
        return;
    }

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
    m_transform.rotation = yawRotation * pitchRotation;

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
        m_transform.position += forward * adjustedMoveSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyS, false))
        m_transform.position -= forward * adjustedMoveSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyA, false))
        m_transform.position -= right * adjustedMoveSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyD, false))
        m_transform.position += right * adjustedMoveSpeed * deltaTime;

    // Handle input for player rotation
    if (inputManager.isKeyDown(Key::KeyQ, false))
        m_transform.position -= up * m_movementSpeed * deltaTime;
    if (inputManager.isKeyDown(Key::KeyE, false))
        m_transform.position += up * m_movementSpeed * deltaTime;

    m_transform.UpdateWorldTransform();
}

Camera* EditorPlayer::getCamera() const
{
    return m_cam;
}
