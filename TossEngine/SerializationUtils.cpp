#include "SerializationUtils.h"

#include "TossEngine.h"


void to_json(json& j, GameObjectPtr const& gameObject)
{
    if (gameObject) {
        j = json{ { "id", gameObject->getId() } };
    }
    else {
        j = nullptr;
    }
}

void from_json(json const& j, GameObjectPtr& gameObject)
{
    if (j.contains("id") && !j["id"].is_null())
    {
        size_t id = j["id"].get<size_t>();
        ScenePtr scene = TossEngine::GetInstance().getCurrentScene();
        if (scene && scene->m_gameObjects.count(id)) {
            gameObject = scene->m_gameObjects.at(id);
        }
    }
}