#pragma once
#include "Resizable.h"

class Game;
class Window;
class Scene;

class TossEditor : public Resizable
{
public:
	TossEditor();
	~TossEditor();

	void run();

	void onResize(Vector2 size) override;
	
	void save();

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
	ProjectSettingsPtr m_projectSettings;
	shared_ptr<Scene> m_currentScene;
	Game* game;

    float m_currentTime = 0.0f;
    float m_previousTime = 0.0f;
};