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
#include "All.h"
#include <reactphysics3d/reactphysics3d.h>

class Game;
class ComponentRegistry;
class Image;
class Camera;

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
     * @param game A pointer to the Game instance, allowing access to game-wide functionalities.
     */
    Scene(const string& filePath);

    Scene(const Scene& other); //Copy Constructor

    /**
     * @brief Destructor for the Scene class.
     */
    ~Scene();

    /**
     * @brief Called when the game is created.
     * Use this method to initialize resources such as textures, meshes, and shaders.
     */
    virtual void onCreate();

    /**
     * @brief Called after onCreate when the game is created.
     * Use this method for additional initialization tasks.
     */
    virtual void onCreateLate();

    /**
     * @brief Updates the graphics rendering mode.
     * Default rendering mode is deferred rendering.
     */
    virtual void onGraphicsUpdate(Camera* cameraToRenderOverride = nullptr);

    /**
     * @brief Called every frame to update the game logic.
     *
     * @param deltaTime The time elapsed since the last update in seconds.
     */
    virtual void onUpdate(float deltaTime);

    /**
     * @brief Called every frame to update the GameObject at a fixed frame rate.
     * Can be overridden by derived classes to implement custom behavior.
     *
     * @param fixedDeltaTime The time elapsed since the last fixed update in seconds.
     */
    virtual void onFixedUpdate(float fixedDeltaTime);

    /**
     * @brief Called every frame after the graphics update/render.
     *
     * @param deltaTime The time elapsed since the last update in seconds.
     */
    virtual void onLateUpdate(float deltaTime);

    /**
     * @brief Called when the window is resized.
     *
     * @param _width The new width of the window.
     * @param _height The new height of the window.
     */
    void onResize(Vector2 size) override;

    /**
     * @brief Called when the game is quitting.
     * Use this method to clean up resources or perform necessary finalization tasks.
     */
    virtual void onQuit();

    void Save();

    rp3d::PhysicsWorld* GetPhysicsWorld();
    rp3d::PhysicsCommon& GetPhysicsCommon();

protected:
    bool m_initilized = false;
    string m_filePath = "";
    UniformData uniformData = {};

    rp3d::PhysicsCommon m_PhysicsCommon;  // Manages physics resources
    rp3d::PhysicsWorld* m_PhysicsWorld;   // Scene-specific physics world

    ShaderPtr ssrQuadLightingShader = nullptr;
    ShaderPtr defaultFullscreenShader = nullptr;
    MaterialPtr m_deferredSSRQMaterial = nullptr;
    MaterialPtr m_postProcessSSRQMaterial = nullptr;

    ShaderPtr m_solidColorMeshShader = nullptr; /**< @brief Shader for rendering solid color meshes. */
    ShaderPtr m_particleSystemShader = nullptr; /**< @brief Shader for rendering particle systems. */
    ShaderPtr m_shadowShader = nullptr; /**< @brief Shader for rendering shadows. */
    ShaderPtr m_shadowInstancedShader = nullptr; /**< @brief Shader for rendering instanced shadows. */
    ShaderPtr m_meshGeometryShader = nullptr; /**< @brief Shader for rendering mesh geometries. */
    // ShaderPtr m_meshLightingShader = nullptr; // Shader for lighting meshes.
    ShaderPtr m_instancedmeshGeometryShader = nullptr; /**< @brief Shader for rendering instanced meshes. */
    ShaderPtr m_terrainGeometryShader = nullptr; /**< @brief Shader for rendering terrain geometries. */
    ShaderPtr m_computeShader = nullptr; /**< @brief Shader for compute operations. */

    unique_ptr<GameObjectManager> m_gameObjectManager; /**< @brief Pointer to the GameObject system managing game entities. */
    FramebufferPtr m_postProcessingFramebuffer; /**< @brief Pointer to the framebuffer for post-processing effects. */
    unique_ptr<Image> m_SSRQ;

    Player* m_player;
};