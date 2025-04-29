/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : TossEditor.h
Description : Main class for the TossEngine editor. Handles scene management, rendering, resource management,
              user interaction, and dynamic recompilation of scripts during development.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

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

/**
 * @class TossEditor
 * @brief Main editor class managing scene editing, resource handling, rendering,
 *        hot-reloading, and file watching in TossEngine.
 */
class TossEditor : public Resizable
{
public:
    TossEditor();

    /**
     * @brief Main loop runner for the editor.
     */
    void run();

    /**
     * @brief Called when the editor window is resized.
     * @param size New window size.
     */
    void onResize(Vector2 size) override;

    /**
     * @brief Called when the window is maximized or restored.
     * @param maximized 1 if maximized, 0 if restored.
     */
    void onMaximize(int maximized) override;

    /**
     * @brief Undoes the last action performed in the editor.
     */
    void Undo();

    /**
     * @brief Saves the current scene and resources to file.
     */
    void Save() const;

    /**
     * @brief Exits the editor and shuts down safely.
     */
    void Exit();

    /**
     * @brief Reloads all scripts (DLL hot-reload) and refreshes scene types.
     */
    void Reload();

    /**
     * @brief Duplicates the currently selected object(s) in the scene.
     */
    void DuplicateSelected();

    /**
     * @brief Deletes the currently selected object(s) from the scene.
     */
    void DeleteSelected();

    /**
     * @brief Creates a brand new empty scene.
     */
    void CreateScene();

    /**
     * @brief Opens an existing scene from the file system.
     */
    void OpenSceneViaFileSystem();

protected:
    /**
     * @brief Updates editor logic before rendering.
     */
    void onUpdateInternal();

    /**
     * @brief Renders the editor view.
     */
    void onRenderInternal();

    /**
     * @brief Updates logic that must happen after the main update.
     */
    void onLateUpdateInternal();

    /**
     * @brief Searches the project directory for scene files.
     */
    void FindSceneFiles();

    /**
     * @brief Initializes core editor subsystems on startup.
     */
    void onCreate();

    /**
     * @brief Additional initialization after onCreate.
     */
    void onCreateLate();

    /**
     * @brief Called before the editor fully quits.
     */
    void onQuit();

    /**
     * @brief Displays a GameObject node recursively in the hierarchy panel.
     * @param gameObject The GameObject to display.
     */
    void ShowGameObjectNode(GameObject* gameObject);

    /**
     * @brief Watches source folders and recompiles scripts on changes.
     */
    void LoadWatchAndCompileScripts();

    /**
     * @brief Safely builds scripts via external tools (e.g., MSBuild).
     */
    void PerformSafeBuild();

    /**
     * @brief Safely reloads compiled DLLs and reconstructs scene types.
     */
    void PerformSafeDllReload();

private:
    // General editor state
    std::atomic<bool> m_editorRunning = true;    //!< Is the editor currently running.
    bool canUpdateInternal = true;               //!< Should the editor currently update.
    std::atomic<bool> requestDllReload = false;  //!< If true, triggers a DLL reload request.
    std::atomic<bool> requestBuild = false;      //!< If true, triggers a build request.

    // Editor configuration
    EditorPreferences editorPreferences;        //!< User preferences for the editor.

    // Scene management
    std::vector<std::string> allSceneFilePaths;  //!< List of available scene files in the project.

    // Script watching and live recompilation
    std::thread scriptWatcherThread;             //!< Thread for monitoring script changes.
    FileWatcher* sourceWatcher = nullptr;        //!< File watcher for the source folder.

    // Project settings
    ProjectSettingsPtr m_projectSettings = nullptr;   //!< Settings specific to the project.
    TossPlayerSettingsPtr m_playerSettings = nullptr; //!< Settings specific to the game player.

    // Editor runtime state
    bool m_gameRunning = false;                  //!< True if the editor is currently playing the game.
    std::shared_ptr<ISelectable> selectedSelectable = nullptr; //!< Currently selected item (GameObject, Resource, etc).
    GameObject* renamingGameObject = nullptr;     //!< GameObject currently being renamed.
    char renameBuffer[256] = "";                  //!< Buffer for renaming GameObjects.

    ResourcePtr resourceBeingRenamed = nullptr;   //!< Resource currently being renamed.
    char iDBuffer[256] = "";                      //!< Temporary buffer for IDs.

    string selectedTypeName = "";                 //!< Currently selected type name (for resource creation).
    bool createResource = false;                  //!< Flag indicating a new resource should be created.

    // Rendering
    ShaderPtr shader;                             //!< Editor-specific shader.

    // Player preview
    unique_ptr<EditorPlayer> m_player = nullptr;  //!< Simulated in-editor player instance.

    // Framebuffers
    FramebufferPtr m_sceneFrameBuffer = nullptr;  //!< Framebuffer used for scene rendering.
    FramebufferPtr m_gameViewFrameBuffer = nullptr; //!< Framebuffer used for game view rendering.

    // Time management
    float m_currentTime = 0.0f;                   //!< Current time since startup.
    float m_previousTime = 0.0f;                  //!< Previous frame's time.
    float m_accumulatedTime = 0.0f;                //!< Time accumulated across frames.

    // Scene management UI
    bool m_openSceneNamePopup = false;            //!< Should the create scene name popup be open.
    std::string m_pendingFolderPath;              //!< Folder path pending scene creation.
    char m_sceneNameBuffer[256] = "";              //!< Buffer for scene name input.

    // Gizmo / Transform editing
    ImGuizmo::OPERATION m_currentManipulateOperation = ImGuizmo::OPERATION::TRANSLATE; //!< Current gizmo operation (Translate/Rotate/Scale).
};