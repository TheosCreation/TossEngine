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
#include "GameObjectFactory.h"
#include <map>
#include <set>

// Forward declarations of classes
class GameObject;
class GraphicsGameObject;
class Camera;
class Scene;

/**
 * @class GameObjectManager
 * @brief A container and controller of the entities for the scene.
 */
class GameObjectManager
{
public:
    /**
     * @brief Default constructor for the GameObjectManager class.
     */
    GameObjectManager();

    /**
     * @brief Constructor for the GameObjectManager class with a scene pointer.
     * @param scene Pointer to the Scene.
     */
    GameObjectManager(Scene* scene);

    /**
     * @brief Destructor for the GameObjectManager class.
     */
    ~GameObjectManager();

    Scene* getScene();

    /**
     * @brief Creates an GameObject of type T.
     * @tparam T The type of GameObject to create.
     * @return A pointer to the created GameObject.
     */
    template <typename T>
    T* createGameObject()
    {
        static_assert(std::is_base_of<GameObject, T>::value, "T must derive from Game Object class");
        auto e = new T();
        if (createGameObjectInternal(e))
            return e;
        return nullptr;
    }

    /**
     * @brief Removes an GameObject from the system.
     * @param GameObject Pointer to the GameObject to remove.
     */
    void removeGameObject(GameObject* GameObject);

    void loadGameObjectsFromFile(const std::string& filePath);
    void saveGameObjectsToFile(const std::string& filePath);

    /**
     * @brief Updates the GameObject system.
     * @param deltaTime The time elapsed since the last update.
     */
    void onUpdate(float deltaTime);

    void onLateUpdate(float deltaTime);
    void onShadowPass(int index);
    void Render(UniformData _data);
    void onGeometryPass(UniformData _data);
    void onTransparencyPass(UniformData _data);
    void onGraphicsUpdate(UniformData _data);

    /**
     * @brief Called every frame to update the GameObject system at a fixed frame rate.
     */
    void onFixedUpdate(float fixedDeltaTime);

    /**
     * @brief Gets all graphics entities.
     * @return A vector of pointers to the graphics entities.
     */
    std::vector<GraphicsGameObject*> getGraphicsEntities() const;

    /**
     * @brief Gets all camera entities.
     * @return A vector of pointers to the camera entities.
     */
    std::vector<Camera*> getCameras() const;

    /**
     * @brief Map of entities categorized by their type ID.
     */
    vector<std::unique_ptr<GameObject>> m_gameObjects;

    void clearGameObjects();

    //move later
    std::unique_ptr<GameObjectFactory> m_gameObjectFactory;

private:
    /**
     * @brief Internal function to create an GameObject.
     * @param GameObject Pointer to the GameObject to create.
     * @param id The type ID of the GameObject.
     * @return True if the GameObject was created successfully, false otherwise.
     */
    bool createGameObjectInternal(GameObject* gameObject);
    

    /**
     * @brief Set of entities scheduled for destruction.
     */
    std::set<GameObject*> m_entitiesToDestroy;

    /**
     * @brief Vector of all graphics entities.
     */
    std::vector<GraphicsGameObject*> m_graphicsObjects;

    /**
     * @brief Vector of all camera entities.
     */
    std::vector<Camera*> m_cameras;

    /**
     * @brief Pointer to the scene instance.
     */
    Scene* m_scene = nullptr;
};