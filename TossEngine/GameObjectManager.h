/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : GameObjectManager.h
Description : Manages GameObjects in a scene. Responsible for creation, update, destruction, serialization,
              prefab instantiation, and object/component type queries.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Utils.h"
#include "Math.h"
#include "Scene.h"
#include "GameObject.h"
#include <map>
#include <set>

// Forward declarations
class Camera;
class Prefab;

/**
 * @class GameObjectManager
 * @brief Container and controller for all GameObjects in a Scene.
 */
class TOSSENGINE_API GameObjectManager
{
public:
    GameObjectManager() = default;
    explicit GameObjectManager(Scene* scene);
    ~GameObjectManager() = default;

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
    void removeGameObject(const GameObject* gameObject);

    // --- Serialization ---

    void loadGameObjects(const json& data);
    json saveGameObjects() const;

    void loadGameObjectsFromFile(const std::string& filePath);
    void saveGameObjectsToFile(const std::string& filePath) const;

    // --- Lifecycle ---

    void onStart() const;
    void onLateStart() const;
    void onUpdate();
    void onUpdateInternal();
    void onLateUpdate() const;
    void onFixedUpdate() const;
    void onDestroy() const;

    void onShadowPass(int index) const;
    void Render(UniformData _data) const;
    void onTransparencyPass(UniformData _data) const;
    void onSkyboxPass(UniformData _data) const;
    void onScreenSpacePass(UniformData _data) const;

    // --- Queries ---

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
        for (auto& [id, gameObject] : m_gameObjects)
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
     * @brief Gets the current scene.
     * @return Pointer to the owning Scene.
     */
    Scene* getScene() const { return m_scene; }

    /**
     * @brief Removes all GameObjects from the scene.
     */
    void clearGameObjects();

    /**
     * @brief Public map of all GameObjects by ID.
     */
    std::unordered_map<size_t, GameObjectPtr> m_gameObjects;

private:
    /**
     * @brief Internal helper to finalize GameObject creation.
     * @param gameObject The object to register.
     * @param name Desired name.
     * @param data Optional JSON for deserialization.
     * @return True if added successfully.
     */
    bool createGameObjectInternal(GameObjectPtr gameObject, const std::string& name = "", const json& data = nullptr);

    /**
     * @brief Ensures a name is unique, appends a suffix if necessary.
     */
    std::string getGameObjectNameAvaliable(std::string currentName);

private:
    size_t m_nextAvailableId = 1; //!< Tracks the next ID to assign.
    std::unordered_set<size_t> m_gameObjectsToDestroy; //!< Objects pending destruction.

    GameObject* selectedGameObject = nullptr;
    Scene* m_scene = nullptr;
};