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
#include "All.h"

class Game;

/**
 * @class Scene
 * @brief Base class representing a game scene, providing essential methods for rendering and updating the game logic.
 */
class Scene
{
public:
    /**
     * @brief Constructor for the Scene class.
     *
     * @param game A pointer to the Game instance, allowing access to game-wide functionalities.
     */
    Scene(Game* game);

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
    virtual void onGraphicsUpdate();

    /**
     * @brief Called every frame to update the game logic.
     *
     * @param deltaTime The time elapsed since the last update in seconds.
     */
    virtual void onUpdate(float deltaTime);

    /**
     * @brief Called every frame to update the entity at a fixed frame rate.
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
    virtual void onResize(int _width, int _height);

    /**
     * @brief Called when the game is quitting.
     * Use this method to clean up resources or perform necessary finalization tasks.
     */
    virtual void onQuit();

protected:
    UniformData uniformData = {};

    ShaderPtr ssrQuadLightingShader = nullptr;
    ShaderPtr defaultFullscreenShader = nullptr;

    ShaderPtr m_solidColorMeshShader = nullptr; /**< @brief Shader for rendering solid color meshes. */
    ShaderPtr m_particleSystemShader = nullptr; /**< @brief Shader for rendering particle systems. */
    ShaderPtr m_shadowShader = nullptr; /**< @brief Shader for rendering shadows. */
    ShaderPtr m_shadowInstancedShader = nullptr; /**< @brief Shader for rendering instanced shadows. */
    ShaderPtr m_meshGeometryShader = nullptr; /**< @brief Shader for rendering mesh geometries. */
    // ShaderPtr m_meshLightingShader = nullptr; // Shader for lighting meshes (currently commented out).
    ShaderPtr m_skyboxGeometryShader = nullptr; /**< @brief Shader for rendering skybox geometries. */
    ShaderPtr m_instancedmeshGeometryShader = nullptr; /**< @brief Shader for rendering instanced meshes. */
    ShaderPtr m_terrainGeometryShader = nullptr; /**< @brief Shader for rendering terrain geometries. */
    ShaderPtr m_computeShader = nullptr; /**< @brief Shader for compute operations. */

    unique_ptr<SkyboxEntity> m_skyBox; /**< @brief Pointer to the skybox instance used in the scene. */
    unique_ptr<GameObjectManager> m_gameObjectManager; /**< @brief Pointer to the entity system managing game entities. */
    FramebufferPtr m_postProcessingFramebuffer; /**< @brief Pointer to the framebuffer for post-processing effects. */
    SSRQuadPtr m_deferredRenderSSRQ;
    SSRQuadPtr m_postProcessSSRQ;

    Game* gameOwner; /**< @brief Pointer to the owner Game instance. */
    vector<MeshEntity*> m_lights; /**< @brief Pointers to the light objects (point and spotlights) in the scene. */

    Player* m_player = nullptr; //Pointer to the player entity

    float m_elapsedSeconds = 0;
    MeshEntity* m_ship = nullptr; //Pointer to the statue entity
    TerrainEntity* m_terrain = nullptr; //Pointer to the terrain entity
};