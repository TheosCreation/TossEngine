/***
DeviousDevs
Auckland
New Zealand
(c) 2026 DeviousDevs
File Name : Scene.h
Description : Base class representing a game scene, providing essential methods for rendering and updating the game logic.
Author : Theo Morris
Mail : theo.morris@outlook.co.nz
**/

#pragma once
#include "Utils.h"
#include "Resizable.h"
#include "Math.h"
#include "GameObject.h"
#include "LightManager.h"
#include <map>
#include <set>
#include <imgui.h>
#include <ranges>

class Image;
class ComponentRegistry;
class Camera;
class Player;
class Physics;
class Window;

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
    ~Scene() override;

    void clean();
    void reload();

    /**
     * @brief Creates a new GameObject of type T.
     * @tparam T The GameObject-derived type.
     * @param name Optional name to assign.
     * @param data Optional JSON data to deserialize.
     * @return Shared pointer to the created GameObject.
     */
    template <typename T>
    std::shared_ptr<T> createGameObject(std::string name = "", json data = nullptr)
    {
        static_assert(std::is_base_of<GameObject, T>::value, "T must derive from GameObject");
        auto go = std::make_shared<T>();
        if (createGameObjectInternal(go, name, data))
            return go;
        return nullptr;
    }

    /**
     * @brief Instantiates a Prefab into the scene.
     */
    GameObjectPtr Instantiate(const PrefabPtr& prefab, Transform* parent = nullptr,
        Vector3 positionalOffset = Vector3(0.0f), Quaternion rotationOffset = Quaternion(), bool hasStarted = true);
    GameObjectPtr Instantiate(const PrefabPtr& prefab, Vector3 position, Quaternion rotation, bool hasStarted = true);

    /**
     * @brief Removes a GameObject from the system.
     * @param gameObject Pointer to the GameObject to remove.
     */
    void deleteGameObject(const GameObject* gameObject, float _delay = 0.0f);

    // --- Serialization ---

    void loadGameObjects(const json& data);
    json saveGameObjects() const;

    void loadGameObjectsFromFile(const std::string& filePath);
    void saveGameObjectsToFile(const std::string& filePath) const;


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
     * @brief Updates the graphics rendering mode.
     * Default rendering mode is deferred rendering.
     */
    virtual void onGraphicsUpdate(Camera* cameraToRenderOverride = nullptr, FramebufferPtr writeToFrameBuffer = nullptr);
    void onDrawGizmos();
    void onShadowPass(int index) const;
    void onGeometryPass(UniformData _data) const;
    void onTransparencyPass(UniformData _data) const;
    void onSkyboxPass(UniformData _data) const;
    void onScreenSpacePass(UniformData _data) const;

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
    Window* getWindow();

    void SetPostProcessMaterial(MaterialPtr material);

    /**
     * @brief Called when the game is quitting.
     * Use this method to clean up resources or perform necessary finalization tasks.
     */
    virtual void onQuit();

    GameObjectPtr getGameObject(size_t id);
    std::vector<Camera*> getCameras() const;

    /**
     * @brief Finds the first object or component of the specified type.
     * @tparam T The type to search for.
     * @return Pointer to the first object or component found.
     */
    template <typename T>
    T* findObjectOfType()
    {
        for (auto& gameObject : m_gameObjects | std::views::values)
        {
            if (!gameObject) continue;

            if (auto go = std::dynamic_pointer_cast<T>(gameObject))
                return go.get();

            for (auto& comp : gameObject->getAllComponents())
            {
                if (auto c = dynamic_cast<T*>(comp.second))
                    return c;
            }
        }
        return nullptr;
    }

    /**
     * @brief Finds all objects or components of the specified type.
     * @tparam T The type to search for.
     * @return Vector of matching pointers.
     */
    template <typename T>
    std::vector<T*> findObjectsOfType()
    {
        std::vector<T*> results;

        for (auto& [id, gameObject] : m_gameObjects)
        {
            if (!gameObject) continue;

            if (T* castedGO = std::dynamic_pointer_cast<T>(gameObject))
                results.push_back(castedGO);

            for (auto& pair : gameObject->getAllComponents())
            {
                if (T* castedComp = dynamic_cast<T*>(pair.second))
                    results.push_back(castedComp);
            }
        }

        return results;
    }

    /**
     * @brief Gets the current skybox texture (if any).
     * @return Pointer to the texture cubemap.
     */
    TextureCubeMapPtr getSkyBoxTexture() const;

    /**
     * @brief Removes all GameObjects from the scene.
     */
    void clearGameObjects();

    /**
     * @brief Public map of all GameObjects by ID.
     */
    std::unordered_map<size_t, GameObjectPtr> m_gameObjects;

    void Save();
    string GetFilePath();

private:
    /**
     * @brief Internal helper to finalize GameObject creation.
     * @param gameObject The object to register.
     * @param name Desired name.
     * @param data Optional JSON for deserialization.
     * @return True if added successfully.
     */
    bool createGameObjectInternal(GameObjectPtr gameObject, const std::string& name = "", const json& data = nullptr, Transform* parent = nullptr, Vector3 position = Vector3::Zero, Quaternion rotation = Quaternion::Identity());

    /**
     * @brief Ensures a name is unique, appends a suffix if necessary.
     */
    std::string getGameObjectNameAvaliable(std::string currentName);

    void DestroyImmediate(size_t _gameobjectId);

protected:
    bool m_initilized = false; //has called create or not
    string m_filePath = "";
    UniformData uniformData = {};

    MaterialPtr m_deferredSSRQMaterial = nullptr;
    MaterialPtr m_postProcessSSRQMaterial = nullptr;

    FramebufferPtr m_postProcessingFramebuffer = nullptr; /**< @brief Pointer to the framebuffer for post-processing effects. */
    unique_ptr<Image> m_SSRQ = nullptr;

    unique_ptr<LightManager> m_lightManager = nullptr;
    Camera* lastCameraToRender = nullptr;

    size_t m_nextAvailableId = 1; //!< Tracks the next ID to assign.
    std::unordered_map<size_t, float> m_objectsToDestroy;

    GameObject* selectedGameObject = nullptr;
    Scene* m_scene = nullptr;
};
