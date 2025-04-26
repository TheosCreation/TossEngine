#pragma once
#include "All.h"

class ScriptLoader;

class TOSSENGINE_API TossEngine
{
public:
    static TossEngine& GetInstance() 
    {
        static TossEngine instance;
        return instance;
    }

    // Delete the copy constructor and assignment operator to prevent copying
    TossEngine(const TossEngine& other) = delete;
    TossEngine& operator=(const TossEngine& other) = delete;

    void Init();
    void LoadGenericResources();
    void TryCreateWindow(Resizable* owner, Vector2 size, const string& windowName, bool maximized = false);
    Window* GetWindow() const;
    ScenePtr getCurrentScene() const;
    GameObjectManager* getGameObjectManager() const;
    void PollEvents();
    void CleanUp();
    void UnLoadScripts() const;
    void LoadScripts() const;
    void ReloadScripts() const;

    void OpenScene(shared_ptr<Scene> _scene, bool callStartMethods = true);
    static float GetTime();

    std::shared_ptr<bool> StartCoroutine(CoroutineTask&& coroutine);

    string openFileDialog(const std::string& filter);

    bool IsDebugMode();
    void SetDebugMode(bool enabled);

private:
    bool isDebugMode = false;
    std::unique_ptr<Window> m_window = nullptr;
    ScriptLoader* m_scriptLoader = nullptr;


    std::atomic<bool> running = false;
    std::thread coroutineThread;

    std::mutex coroutineMutex;
    std::queue<CoroutineTask> coroutineQueue;
    std::condition_variable coroutineCondition;

    void CoroutineRunner();

    shared_ptr<Scene> m_currentScene = nullptr;

    /**
     * @brief Private constructor to prevent external instantiation.
     */
    TossEngine() = default;

    /**
     * @brief Private destructor to prevent external deletion.
     */
    ~TossEngine() = default;
};




inline void to_json(json& j, GameObjectPtr const& gameObject) {
    if (gameObject)
    {
        j = json{ { "id", gameObject->getId() } };
    }
    else {
        j = nullptr;
    }
}

inline void from_json(json const& j, GameObjectPtr& gameObject)
{
    if (j.contains("id") && !j["id"].is_null())
    {
        size_t id = j["id"].get<size_t>();
        GameObjectManager* mgr = TossEngine::GetInstance().getGameObjectManager();
        if (mgr && mgr->m_gameObjects.count(id)) {
            gameObject = mgr->m_gameObjects.at(id);
        }
    }
}