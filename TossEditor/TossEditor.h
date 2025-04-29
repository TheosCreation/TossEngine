/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : TossEditor.h
Description : Main class for the TossEngine editor. Manages scene editing, rendering, resource management,
              input handling, live script recompilation, and editor UI.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Resizable.h"
#include "EditorPreferences.h"
#include <ImGuizmo.h>
#include "TossEngine.h"

// Forward declarations for internal engine types
class Window;
class Scene;
class EditorPlayer;
class GameObject;
class Component;
class ISelectable;
class FileWatcher;

/**
 * @class TossEditor
 * @brief Core class for TossEngine's editor. Handles editor state, game view, object management,
 *        resource editing, script hot-reload, and GUI systems.
 */
class TossEditor : public Resizable
{
public:
    TossEditor();
    ~TossEditor();

    /**
     * @brief Starts the main editor loop.
     */
    void run();

    /**
     * @brief Called when the editor window is resized.
     */
    void onResize(Vector2 size) override;

    /**
     * @brief Called when the editor window is maximized or restored.
     */
    void onMaximize(int maximized) override;

    /**
     * @brief Undo the last editor operation (if supported).
     */
    void Undo();

    /**
     * @brief Saves the current scene and resources to file.
     */
    void Save() const;

    /**
     * @brief Shuts down and exits the editor.
     */
    void Exit();

    /**
     * @brief Reloads the runtime scripts via hot-swapping the game DLL.
     */
    void Reload();

    /**
     * @brief Duplicates the currently selected GameObject.
     */
    void DuplicateSelected();

    /**
     * @brief Deletes the currently selected GameObject or component.
     */
    void DeleteSelected();

    /**
     * @brief Opens an existing scene via file selection.
     */
    void OpenSceneViaFileSystem();

    /**
     * @brief Creates a new blank scene.
     */
    void CreateScene();

protected:
    /**
     * @brief Called before rendering each frame.
     */
    void onUpdateInternal();

    /**
     * @brief Called when rendering the editor interface.
     */
    void onRenderInternal();

    /**
     * @brief Called after logic and rendering to finish frame updates.
     */
    void onLateUpdateInternal();

    /**
     * @brief Searches for available scene files in the project directory.
     */
    void FindSceneFiles();

    /**
     * @brief Called on editor initialization.
     */
    void onCreate();

    /**
     * @brief Additional setup after core initialization is complete.
     */
    void onCreateLate();

    /**
     * @brief Final cleanup before quitting the editor.
     */
    void onQuit();

    /**
     * @brief Recursively draws a GameObject node in the scene hierarchy panel.
     */
    void ShowGameObjectNode(GameObject* gameObject);

    /**
     * @brief Initializes file watchers and triggers automatic recompilation on source changes.
     */
    void LoadWatchAndCompileScripts();

    /**
     * @brief Performs a safe external build of the game's script DLL.
     */
    void PerformSafeBuild();

    /**
     * @brief Reloads the built DLL, deserializes scenes, and refreshes types.
     */
    void PerformSafeDllReload();

private:
    // --- State control ---
    std::atomic<bool> m_editorRunning = true;    //!< Controls the editor main loop.
    bool canUpdateInternal = true;               //!< Controls internal update logic.
    std::atomic<bool> requestDllReload = false;  //!< Triggers DLL hot-reload request.
    std::atomic<bool> requestBuild = false;      //!< Triggers external build request.

    // --- Editor preferences ---
    EditorPreferences editorPreferences;         //!< Editor user settings and configuration.

    // --- Scene discovery ---
    std::vector<std::string> allSceneFilePaths;  //!< All discovered scene file paths.

    // --- Script watching ---
    std::thread scriptWatcherThread;             //!< Thread for watching source file changes.
    FileWatcher* sourceWatcher = nullptr;        //!< Active file system watcher for scripts.

    // --- Project configuration ---
    ProjectSettingsPtr m_projectSettings = nullptr;     //!< Serialized project-wide settings.
    TossPlayerSettingsPtr m_playerSettings = nullptr;   //!< Serialized runtime player settings.

    // --- Runtime state ---
    bool m_gameRunning = false;                         //!< True if the game is currently playing in the editor.
    std::shared_ptr<ISelectable> selectedSelectable = nullptr; //!< Selected resource, GameObject, or component.
    GameObject* renamingGameObject = nullptr;           //!< GameObject currently being renamed.
    char renameBuffer[256] = "";                        //!< Rename input buffer.
    ResourcePtr resourceBeingRenamed = nullptr;         //!< Resource currently being renamed.

    // --- Resource/UI creation state ---
    char iDBuffer[256] = "";                            //!< Temporary input buffer for IDs.
    std::string selectedTypeName = "";                  //!< Selected resource type name.
    bool createResource = false;                        //!< True if resource creation is requested.

    // --- Editor shader ---
    ShaderPtr shader;                                   //!< Editor-specific ImGui/viewport shader.

    // --- In-editor player simulation ---
    std::unique_ptr<EditorPlayer> m_player = nullptr;   //!< Simulated player for runtime preview.

    // --- Render targets ---
    FramebufferPtr m_sceneFrameBuffer = nullptr;        //!< Framebuffer for scene view.
    FramebufferPtr m_gameViewFrameBuffer = nullptr;     //!< Framebuffer for game view preview.

    // --- Time tracking ---
    float m_currentTime = 0.0f;                         //!< Current time since launch.
    float m_previousTime = 0.0f;                        //!< Time in the previous frame.
    float m_accumulatedTime = 0.0f;                     //!< Time carried over for fixed updates.

    // --- Scene creation popup ---
    bool m_openSceneNamePopup = false;                  //!< True if the "Create Scene" popup is open.
    std::string m_pendingFolderPath;                    //!< Target folder for the new scene.
    char m_sceneNameBuffer[256] = "";                   //!< Input buffer for new scene name.

    // --- Transform manipulation ---
    ImGuizmo::OPERATION m_currentManipulateOperation = ImGuizmo::TRANSLATE; //!< Current active gizmo mode.
};