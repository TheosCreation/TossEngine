#pragma once
#include "Resizable.h"
#include "EditorPreferences.h"
#include "TossEngine.h"

class Window;
class Scene;
class EditorPlayer;
class GameObject;
class Component;
class ISelectable;
class FileWatcher;

class TossEditor : public Resizable
{
public:
	TossEditor();
	~TossEditor();

	void run();

	void onResize(Vector2 size) override;
    void onMaximize(int maximized) override;
	
	void Undo();
	void Save();
    void Exit();

    void DeleteSelected();

    void OpenSceneViaFileSystem();
    void OpenScene(shared_ptr<Scene> _scene);

protected:
    void onUpdateInternal();
    void onLateUpdateInternal();
    void FindSceneFiles();
    /**
     * @brief Called when the editor is created.
     */
    void onCreate();

    /**
     * @brief Called when the editor is created and after onCreate.
     */
    void onCreateLate();

    /**
     * @brief Called when the editor is quitting.
     */
    void onQuit();

    void ShowGameObjectNode(GameObject* gameObject);

    void LoadWatchAndCompileScripts();
    void PerformSafeBuild();
    void PerformSafeDllReload();

private:
    std::atomic<bool> m_editorRunning = true;
    bool canUpdateInternal = true;
    std::atomic<bool> requestDllReload = false;
    std::atomic<bool> requestBuild = false;

    EditorPreferences editorPreferences;
    std::vector<std::string> allSceneFilePaths = std::vector<std::string>();
    std::thread scriptWatcherThread;

    FileWatcher* sourceWatcher = nullptr;
	ProjectSettingsPtr m_projectSettings = nullptr;
	TossPlayerSettingsPtr m_playerSettings = nullptr;
	shared_ptr<Scene> m_currentScene = nullptr;
	bool m_gameRunning = false;
    ISelectable* selectedSelectable = nullptr;
    GameObject* renamingGameObject = nullptr;
    char renameBuffer[256] = "";
    ResourcePtr resourceBeingRenamed = nullptr;

    std::string shaderVertPath;
    std::string shaderFragPath;
    char shaderIDBuffer[256] = "";
    bool openShaderPopupNextFrame = false;

    std::string meshFilepath;
    char meshIDBuffer[256] = "";
    bool openMeshPopupNextFrame = false;

    char physicsMaterialIDBuffer[256] = "";
    bool openPhysicsMaterialNextFrame = false;

    unique_ptr<EditorPlayer> m_player = nullptr;
    FramebufferPtr m_sceneFrameBuffer = nullptr;
    FramebufferPtr m_gameViewFrameBuffer = nullptr;

    float m_currentTime = 0.0f;
    float m_previousTime = 0.0f;
    float m_previousFixedUpdateTime = 0; //The previous fixedUpdate frame time
    const float m_fixedTimeStep = 1.0f / 60.0f; // Fixed time step (60 FPS)
    float m_accumulatedTime = 0; //The current frame's time
};