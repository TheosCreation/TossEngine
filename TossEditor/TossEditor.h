#pragma once
#include "Resizable.h"
#include "EditorPreferences.h"
#include <ImGuizmo.h>
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
	void Save() const;
    void Exit();
    void Reload();
    void DuplicateSelected();

    void DeleteSelected();

    void OpenSceneViaFileSystem();
    void CreateScene();

protected:
    void onUpdateInternal();
    void onRenderInternal();
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
	bool m_gameRunning = false;
    std::shared_ptr<ISelectable> selectedSelectable = nullptr;
    GameObject* renamingGameObject = nullptr;
    char renameBuffer[256] = "";
    ResourcePtr resourceBeingRenamed = nullptr;

    char iDBuffer[256] = "";
    string selectedTypeName = "";
    bool createResource = false;

    ShaderPtr shader;

    unique_ptr<EditorPlayer> m_player = nullptr;
    FramebufferPtr m_sceneFrameBuffer = nullptr;
    FramebufferPtr m_gameViewFrameBuffer = nullptr;

    float m_currentTime = 0.0f;
    float m_previousTime = 0.0f;
    float m_accumulatedTime = 0; //The current frame's time

    bool m_openSceneNamePopup = false;
    std::string m_pendingFolderPath;
    char m_sceneNameBuffer[256] = "";

    ImGuizmo::OPERATION m_currentManipulateOperation = ImGuizmo::OPERATION::TRANSLATE;
};