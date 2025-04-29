/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : TossEngine.h
Description : Core singleton class managing windowing, scenes, scripting, events, coroutines, and editor state for TossEngine.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once
#include "All.h"

class ScriptLoader;

/**
 * @class TossEngine
 * @brief Core singleton engine class responsible for initializing and managing the TossEngine runtime.
 */
class TOSSENGINE_API TossEngine
{
public:
    /**
     * @brief Retrieves the singleton instance of TossEngine.
     * @return Reference to TossEngine instance.
     */
    static TossEngine& GetInstance()
    {
        static TossEngine instance;
        return instance;
    }

    TossEngine(const TossEngine& other) = delete;
    TossEngine& operator=(const TossEngine& other) = delete;

    /**
     * @brief Initializes TossEngine systems (window, graphics, resources, etc).
     */
    void Init();

    /**
     * @brief Loads default engine resources.
     */
    void LoadGenericResources();

    /**
     * @brief Creates a window for rendering and interaction.
     * @param owner The Resizable owner (for callbacks).
     * @param size Initial window size.
     * @param windowName Title of the window.
     * @param maximized Whether the window should start maximized.
     */
    void TryCreateWindow(Resizable* owner, Vector2 size, const string& windowName, bool maximized = false);

    /**
     * @brief Retrieves the currently active window.
     * @return Pointer to the Window object.
     */
    Window* GetWindow() const;

    /**
     * @brief Gets the current active scene.
     * @return Shared pointer to the current scene.
     */
    ScenePtr getCurrentScene() const;

    /**
     * @brief Gets the active GameObject manager for the current scene.
     * @return Pointer to the GameObjectManager.
     */
    GameObjectManager* getGameObjectManager() const;

    /**
     * @brief Polls platform events (e.g., input, window close).
     */
    void PollEvents();

    /**
     * @brief Cleans up TossEngine systems and resources before shutdown.
     */
    void CleanUp();

    /**
     * @brief Unloads all loaded scripts from the scripting system.
     */
    void UnLoadScripts() const;

    /**
     * @brief Loads all scripts into the scripting system.
     */
    void LoadScripts() const;

    /**
     * @brief Reloads scripts dynamically (hot reload).
     */
    void ReloadScripts() const;

    /**
     * @brief Updates internal engine logic (coroutines, pending scene changes).
     */
    void onUpdateInternal();

    /**
     * @brief Opens and sets a new active scene.
     * @param _scene Pointer to the new scene.
     * @param callStartMethods If true, onStart and related lifecycle methods are called.
     */
    void OpenScene(std::shared_ptr<Scene> _scene, bool callStartMethods = true);

    /**
     * @brief Opens and loads a scene from a file path.
     * @param sceneName Name of the scene file to open.
     */
    void OpenScene(const string& sceneName);

    /**
     * @brief Requests the engine to quit and shutdown.
     */
    void Quit();

    /**
     * @brief Sets player-specific runtime settings.
     * @param playerSettings Pointer to the TossPlayerSettings structure.
     */
    void SetPlayerSettings(TossPlayerSettings* playerSettings);

    /**
     * @brief Gets the current application running time in seconds.
     * @return Time in seconds.
     */
    static float GetTime();

    /**
     * @brief Starts a coroutine task to be updated asynchronously.
     * @param coroutine The CoroutineTask to start.
     * @return Shared pointer to a bool that can be used to check coroutine completion.
     */
    std::shared_ptr<bool> StartCoroutine(CoroutineTask&& coroutine);

    /**
     * @brief Opens a file picker dialog to select a file.
     * @param filter Filter for file types (e.g., "*.json").
     * @return Path to the selected file, or empty string if cancelled.
     */
    static string openFileDialog(const std::string& filter);

    /**
     * @brief Opens a folder picker dialog to select a directory.
     * @return Path to the selected folder, or empty string if cancelled.
     */
    static string openFolderDialog();

    /**
     * @brief Checks if the engine is running in debug mode.
     * @return True if debug mode is enabled.
     */
    bool IsDebugMode();

    /**
     * @brief Enables or disables debug mode for the engine.
     * @param enabled True to enable, false to disable.
     */
    void SetDebugMode(bool enabled);

private:
    bool isDebugMode = false;
    std::unique_ptr<Window> m_window = nullptr;
    ScriptLoader* m_scriptLoader = nullptr;
    TossPlayerSettings* m_currentPlayerSettings = nullptr;

    std::atomic<bool> running = false;
    std::thread coroutineThread;
    std::mutex coroutineMutex;
    std::queue<CoroutineTask> coroutineQueue;
    std::condition_variable coroutineCondition;

    ScenePtr m_currentScene = nullptr;
    ScenePtr sceneToOpen = nullptr;

    /**
     * @brief Internal coroutine thread runner.
     */
    void CoroutineRunner();

    TossEngine() = default;
    ~TossEngine() = default;
};

// --- JSON support for GameObjectPtr (serialization/deserialization) ---

/**
 * @brief Serializes a GameObjectPtr to JSON.
 */
inline void to_json(json& j, GameObjectPtr const& gameObject) {
    if (gameObject) {
        j = json{ { "id", gameObject->getId() } };
    }
    else {
        j = nullptr;
    }
}

/**
 * @brief Deserializes a GameObjectPtr from JSON.
 */
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