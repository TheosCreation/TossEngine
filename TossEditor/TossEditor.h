#pragma once
#include "Resizable.h"

class Game;
class Window;
class Scene;
class EditorPlayer;
class GameObject;
class Component;

class TossEditor : public Resizable
{
public:
	TossEditor();
	~TossEditor();

	void run();

	void onResize(Vector2 size) override;
	
	void Undo();
	void Save();
    void Exit();

    void DeleteSelected();

    void OpenSceneViaFileSystem();
    void OpenScene(shared_ptr<Scene> _scene);

protected:
    void onUpdateInternal();
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

private:
	ProjectSettingsPtr m_projectSettings = nullptr;
	shared_ptr<Scene> m_currentScene = nullptr;
	Game* m_game = nullptr;
    GameObject* selectedGameObject = nullptr; 
    Component* selectedComponent = nullptr;
    GameObject* renamingGameObject = nullptr;
    char renameBuffer[256] = "";

    unique_ptr<EditorPlayer> m_player = nullptr;
    FramebufferPtr m_sceneViewFrameBuffer = nullptr;
    FramebufferPtr m_gameViewFrameBuffer = nullptr;

    float m_currentTime = 0.0f;
    float m_previousTime = 0.0f;
    float m_previousFixedUpdateTime = 0; //The previous fixedUpdate frame time
    const float m_fixedTimeStep = 1.0f / 60.0f; // Fixed time step (60 FPS)
    float m_accumulatedTime = 0; //The current frame's time
};