#pragma once
#include "Resizable.h"

class Game;
class Scene;

class TossPlayer : public Resizable
{
public:
    TossPlayer();
	~TossPlayer();

	void run();

	void onResize(Vector2 size) override;

protected:
    void onUpdateInternal();
    /**
     * @brief Called when the player is created.
     */
    void onCreate();

    /**
     * @brief Called when the player is created and after onCreate.
     */
    void onCreateLate();

    /**
     * @brief Called when the player is quitting.
     */
    void onQuit();

private:
    TossPlayerSettingsPtr m_playerSettings = nullptr;
	Game* m_game = nullptr;
    bool abort = false;

    float m_currentTime = 0.0f;
    float m_previousTime = 0.0f;
    float m_previousFixedUpdateTime = 0; //The previous fixedUpdate frame time
    const float m_fixedTimeStep = 1.0f / 60.0f; // Fixed time step (60 FPS)
    float m_accumulatedTime = 0; //The current frame's time
};