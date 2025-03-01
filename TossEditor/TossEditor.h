#pragma once
#include "Resizable.h"

class Game;
class Window;
class Scene;
class EditorPlayer;

class TossEditor : public Resizable
{
public:
	TossEditor();
	~TossEditor();

	void run();

	void onResize(Vector2 size) override;
	
	void Save();

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

private:
	ProjectSettingsPtr m_projectSettings = nullptr;
	shared_ptr<Scene> m_currentScene = nullptr;
	Game* m_game = nullptr;
    unique_ptr<EditorPlayer> m_player = nullptr;

    float m_currentTime = 0.0f;
    float m_previousTime = 0.0f;
    float m_previousFixedUpdateTime = 0; //The previous fixedUpdate frame time
    const float m_fixedTimeStep = 1.0f / 60.0f; // Fixed time step (60 FPS)
    float m_accumulatedTime = 0; //The current frame's time
};