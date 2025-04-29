#pragma once
#include "Utils.h"
#include "GameObject.h"

class Camera;
class TossEditor;

class EditorPlayer : public GameObject
{
public:
    /**
     * @brief Constructor for the Player class.
     */
    EditorPlayer(TossEditor* editor);

    /**
     * @brief Destructor for the Player class.
     */
    ~EditorPlayer();

    void onCreate() override;

    /**
     * @brief Called every frame to update the editor player.
     * @param deltaTime The time elapsed since the last update.
     */
    void Update(float deltaTime);

    Camera* getCamera() const;

private:
    TossEditor* Editor = nullptr;
    float m_elapsedSeconds = 0.0f; // Elapsed time in seconds

    float m_minSpeed = 1.0f; // 
    float m_maxSpeed = 500.0f; // 
    float m_movementSpeed = 35.0f; // Movement speed of the movable object
    float m_speedAdjustmentFactor = 2.0f; // 
    float m_yaw = 0.0f; // Yaw angle for rotation of the camera
    float m_pitch = 0.0f; // Pitch angle for rotation of the camera
    float m_minFov = 1.0f; // Minimum fov for the camera
    float m_fov = 90.0f; // Fov for the camera
    float m_maxFov = 120.0f; // Maximum fov for the camera
    float m_zoomSpeed = 0.5f; // Speed of zooming
    bool m_wireframeMode = false; // Flag for wireframe mode

    Camera* m_cam = nullptr; // Pointer to the main camera
    //SoundPtr fireSound = nullptr;
    //unique_ptr<Camera> m_uiCamera = nullptr; // Pointer to the UI camera
};

