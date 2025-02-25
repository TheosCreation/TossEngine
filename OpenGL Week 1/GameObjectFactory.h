#pragma once
#include "Utils.h"
#include "GameObject.h"


class GameObjectFactory
{
public:
    using CreateGameObjectFunc = std::function<GameObject*()>; 
    GameObjectFactory() = default; // Default constructor, no need to initialize a map


    template <typename T>
    void registerGameObject()
    {
        static_assert(std::is_base_of<GameObject, T>::value, "T must derive from GameObject class");
        std::string typeName = getClassName(typeid(T));  // Ensure consistent type name usage

        std::cout << "Registering: " << typeName << std::endl; // Debug
        m_GameObjectCreators.emplace(typeName, []() -> GameObject* { return new T(); });
    }

    GameObject* createGameObject(const std::string& typeName)
    {
        std::cout << "Creating GameObject of type: " << typeName << std::endl;
        auto it = m_GameObjectCreators.find(typeName);

        if (it == m_GameObjectCreators.end()) {
            std::cout << "GameObject type not found!\n";
            return nullptr;
        }

        return it->second(); // Call stored function
    }

private:
    std::map<std::string, CreateGameObjectFunc> m_GameObjectCreators;
};