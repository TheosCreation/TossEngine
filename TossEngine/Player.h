/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Player.h
Description : Player class is an GameObject that can be adjusted by the end user
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "InputManager.h"
#include "LightManager.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "Camera.h"

/**
 * @class Player
 * @brief An GameObject that can be adjusted by the end user.
 */
class Player : public GameObject
{
public:
    /**
     * @brief Constructor for the Player class.
     */
    Player();

    /**
     * @brief Destructor for the Player class.
     */
    ~Player();

    /**
     * @brief Called when the player GameObject is created.
     */
    virtual void onCreate();

    /**
     * @brief Called every frame to update the player GameObject.
     * @param deltaTime The time elapsed since the last update.
     */
    virtual void onUpdate(float deltaTime);

    Camera* getCamera();

private:
    float m_elapsedSeconds = 0.0f; // Elapsed time in seconds
    
    float m_minSpeed = 1.0f; // 
    float m_maxSpeed = 500.0f; // 
    float m_movementSpeed = 50.0f; // Movement speed of the movable object
    float m_speedAdjustmentFactor = 2.0f; // 
    float m_yaw = 0.0f; // Yaw angle for rotation of the camera
    float m_pitch = 0.0f; // Pitch angle for rotation of the camera
    float m_minFov = 1.0f; // Minimum fov for the camera
    float m_fov = 90.0f; // Fov for the camera
    float m_maxFov = 120.0f; // Maximum fov for the camera
    float m_zoomSpeed = 0.5f; // Speed of zooming
    bool m_playMode = true; // Flag for locking the cursor
    bool m_wireframeMode = false; // Flag for wireframe mode

    Camera* m_cam = nullptr; // Pointer to the main camera
    SoundPtr fireSound = nullptr;
    //unique_ptr<Camera> m_uiCamera = nullptr; // Pointer to the UI camera
};