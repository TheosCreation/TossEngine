/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Scene.h
Description : Base class representing a game scene, providing essential methods for rendering and updating the game logic.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Utils.h"
#include "Resizable.h"
#include <imgui.h>

class ComponentRegistry;
class GameObjectManager;
class Image;
class Camera;
class Player;
class LightManager;
class Physics;

/**
 * @class Scene
 * @brief Base class representing a game scene, providing essential methods for rendering and updating the game logic.
 */
class TOSSENGINE_API Scene : public Resizable
{
public:
    /**
     * @brief Constructor for the Scene class.
     *
     * @param filePath A filepath to save the scene to when saving or loading
     */
    Scene(const string& filePath);

    Scene(const Scene& other); //Copy Constructor

    /**
     * @brief Destructor for the Scene class.
     */
    ~Scene();

    void reload();

    /**
     * @brief Called when the scene is created.
     * Use this method to initialize resources such as textures, meshes, and shaders.
     */
    virtual void onCreate();

    /**
     * @brief Called when the game is started right before the first update frame.
     */
    virtual void onStart(); 
    
    /**
     * @brief Called right after start.
     */
    virtual void onLateStart();

    /**
     * @brief Called after onCreate when the game is created.
     * Use this method for additional initialization tasks.
     */
    virtual void onCreateLate();

    /**
     * @brief Updates the graphics rendering mode.
     * Default rendering mode is deferred rendering.
     */
    virtual void onGraphicsUpdate(Camera* cameraToRenderOverride = nullptr, FramebufferPtr writeToFrameBuffer = nullptr);

    /**
     * @brief Called every frame to update the game logic.
     *
     * @param deltaTime The time elapsed since the last update in seconds.
     */
    virtual void onUpdate();
    virtual void onUpdateInternal();

    /**
     * @brief Called every frame to update the GameObject at a fixed frame rate.
     * Can be overridden by derived classes to implement custom behavior.
     *
     */
    virtual void onFixedUpdate();

    /**
     * @brief Called every frame after the graphics update/render.
     *
     * @param deltaTime The time elapsed since the last update in seconds.
     */
    virtual void onLateUpdate();

    /**
     * @brief Called when the window is resized.
     *
     * @param _width The new width of the window.
     * @param _height The new height of the window.
     */
    void onResize(Vector2 size) override;

    LightManager* getLightManager();
    GameObjectManager* getObjectManager();

    /**
     * @brief Called when the game is quitting.
     * Use this method to clean up resources or perform necessary finalization tasks.
     */
    virtual void onQuit();

    void Save();
    string GetFilePath();

protected:
    bool m_initilized = false;
    string m_filePath = "";
    UniformData uniformData = {};

    MaterialPtr m_deferredSSRQMaterial = nullptr;
    MaterialPtr m_postProcessSSRQMaterial = nullptr;

    unique_ptr<GameObjectManager> m_gameObjectManager; /**< @brief Pointer to the GameObject system managing game objects. */
    FramebufferPtr m_postProcessingFramebuffer; /**< @brief Pointer to the framebuffer for post-processing effects. */
    unique_ptr<Image> m_SSRQ;

    unique_ptr<LightManager> m_lightManager = nullptr;
};