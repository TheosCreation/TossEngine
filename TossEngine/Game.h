/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Game.h
Description : Game class that controls the order the graphics engine and internal systems performs tasks
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include <iostream>
#include "InputManager.h"
#include "GraphicsEngine.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "LightManager.h"
#include "Scene.h"
#include "ShadowMap.h"
#include "Player.h"
#include "MonoIntegration.h"

// Forward declarations of classes
class Window;
class GameObjectManager;
class SkyboxGameObject;
class GeometryBuffer;
class Framebuffer;
class Camera;

/**
 * @class Game
 * @brief Controls the order in which the graphics engine and internal systems performs tasks.
 */
class TOSSENGINE_API Game : public Resizable
{
public:
    /**
     * @brief Constructor for the Game class.
     */
    Game();

    /**
     * @brief Destructor for the Game class.
     */
    ~Game();

    /**
     * @brief Runs the game loop.
     */
    void run();

    /**
     * @brief Quits the game.
     */
    void quit();
    void onResize(Vector2 size) override;

    void SetScene(shared_ptr<Scene> _scene);

    /**
     * @brief Gets the Window instance.
     * @return A pointer to the Window instance.
     */
    Window* getWindow();

    float GetCurrentTime();

    MeshPtr getCubeMesh();

    MeshPtr getSphereMesh();

    bool getIsRunning();

    //void SetFullScreenShader(ShaderPtr _shader = nullptr, Texture2DPtr _texture = nullptr);

protected:
    /**
     * @brief Called when the game is created.
     */
    virtual void onCreate();

    /**
     * @brief Called when the game is created and after onCreate.
     */
    virtual void onCreateLate();

    /**
     * @brief Called when the game is quitting.
     */
    virtual void onQuit();

private:
    /**
     * @brief Internal function to update the game.
     */
    void onUpdateInternal();

protected:
    bool m_isRunning = false; //Indicates whether the game is running
    std::unique_ptr<Window> m_display; //Pointer to the window instance
    //std::unique_ptr<QuadGameObject> m_canvasQuad; //Pointer to the framebuffer instance

    float m_previousTime = 0; //The previous frame's time
    float m_previousFixedUpdateTime = 0; //The previous fixedUpdate frame time
    float m_currentTime = 0; //The current frame's time
    const float m_fixedTimeStep = 1.0f / 60.0f; // Fixed time step (60 FPS)
    float m_accumulatedTime = 0; //The current frame's time
    float m_scale = 0; //The scale factor for time

    //collection of useful meshes
    MeshPtr m_cubeMesh;
    MeshPtr m_sphereMesh;

    shared_ptr<Scene> m_currentScene;

    ProjectSettingsPtr m_projectSettings;
};