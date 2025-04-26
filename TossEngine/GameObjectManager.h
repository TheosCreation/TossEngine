/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : GameObjectManager.h
Description : GameObject system is a container and controller of the entities for the game
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Utils.h"
#include "Math.h"
#include "Scene.h"
#include "GameObject.h"
#include <map>
#include <set>

// Forward declarations of classes
class Camera;
class Prefab;

/**
 * @class GameObjectManager
 * @brief A container and controller of the entities for the scene.
 */
class TOSSENGINE_API GameObjectManager
{
public:
    /**
     * @brief Default constructor for the GameObjectManager class.
     */
    GameObjectManager() = default;

    /**
     * @brief Constructor for the GameObjectManager class with a scene pointer.
     * @param scene Pointer to the Scene.
     */
    GameObjectManager(Scene* scene);

    GameObjectManager(const GameObjectManager& other);

    /**
     * @brief Destructor for the GameObjectManager class.
     */
    ~GameObjectManager() = default;

    Scene* getScene() const;

    /**
     * @brief Creates an GameObject of type T.
     * @tparam T The type of GameObject to create.
     * @return A pointer to the created GameObject.
     */
    template <typename T>
    std::shared_ptr<T> createGameObject(string name = "", json data = nullptr)
    {
        static_assert(std::is_base_of<GameObject, T>::value, "T must derive from Game Object class");
        auto go = std::make_shared<T>();
        if (createGameObjectInternal(go, name, data))
            return go;
        return nullptr;
    }

    GameObjectPtr Instantiate(const PrefabPtr& prefab, Transform* parent = nullptr, Vector3 positionalOffset = Vector3(0.0f), Quaternion rotationOffset = Quaternion(), bool hasStarted = true);
    GameObjectPtr Instantiate(const PrefabPtr& prefab, Vector3 position, Quaternion rotation, bool hasStarted = true);

    /**
     * @brief Removes an GameObject from the system.
     * @param GameObject Pointer to the GameObject to remove.
     */
    void removeGameObject(const GameObject* gameObject);

    void loadGameObjects(const json& data);
    json saveGameObjects() const;
    GameObjectPtr getGameObject(size_t id);

    void loadGameObjectsFromFile(const std::string& filePath);
    void saveGameObjectsToFile(const std::string& filePath) const;

    void onStart() const;
    void onLateStart() const;

    /**
     * @brief Updates the GameObject system.
     * @param deltaTime The time elapsed since the last update.
     */
    void onUpdate();
    void onUpdateInternal();

    void onLateUpdate() const;
    void onShadowPass(int index) const;
    void Render(UniformData _data) const;
    void onTransparencyPass(UniformData _data) const;
    void onSkyboxPass(UniformData _data) const;
    void onScreenSpacePass(UniformData _data) const;

    /**
     * @brief Called every frame to update the GameObject system at a fixed frame rate.
     */
    void onFixedUpdate() const;
    void onDestroy() const;

    /**
     * @brief Gets all camera entities.
     * @return A vector of pointers to the camera entities.
     */
    std::vector<Camera*> getCameras() const;

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

    template <typename T>
    std::vector<T*> findObjectsOfType()
    {
        std::vector<T*> results;

        for (auto& [id, gameObject] : m_gameObjects)
        {
            // Check the GameObject itself
            if (T* castedGO = std::dynamic_pointer_cast<T>(gameObject))
                results.push_back(castedGO);

            // Check its components
            for (auto& pair : gameObject->getAllComponents())
            {
                if (T* castedComp = dynamic_cast<T>(pair.second))
                    results.push_back(castedComp);
            }
        }

        return results;
    }
    
    TextureCubeMapPtr getSkyBoxTexture() const;

    /**
    * @brief Map of game objects categorized by type ID.
    */
    std::unordered_map<size_t, GameObjectPtr> m_gameObjects;


    void clearGameObjects();

private:
    size_t m_nextAvailableId = 1;

    /**
     * @brief Internal function to create an GameObject.
     * @param GameObject Pointer to the GameObject to create.
     * @param id The type ID of the GameObject.
     * @return True if the GameObject was created successfully, false otherwise.
     */
    bool createGameObjectInternal(GameObjectPtr gameObject, const string& name = "", const json& data = nullptr);
    string getGameObjectNameAvaliable(string currentName);
    

    /**
     * @brief Set of entities scheduled for destruction.
     */
    std::unordered_set<size_t> m_gameObjectsToDestroy;
    GameObject* selectedGameObject = nullptr;

    /**
     * @brief Vector of all camera entities.
     */
    //std::vector<Camera*> m_cameras;

    /**
     * @brief Pointer to the scene instance.
     */
    Scene* m_scene = nullptr;
};