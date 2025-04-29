/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : InputManager.h
Description : Handles keyboard, mouse, and play mode input states for TossEngine.
              Supports real-time input querying, cursor locking, and state transitions.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Utils.h"
#include "Rect.h"
#include "Window.h"
#include <map>

/**
 * @class InputManager
 * @brief Singleton class that manages real-time user input, including keyboard, mouse, and cursor control.
 */
class TOSSENGINE_API InputManager
{
public:
    /**
     * @brief Provides access to the singleton instance of InputManager.
     * @return Reference to the singleton instance.
     */
    static InputManager& GetInstance()
    {
        static InputManager instance; // Guaranteed to be destroyed, instantiated on first use.
        return instance;
    }

    // Deleted copy constructor and assignment to enforce singleton pattern
    InputManager(const InputManager&) = delete;
    void operator=(const InputManager&) = delete;

    /**
     * @brief Initializes input manager with project settings (Editor usage).
     * @param projectSettings The project settings.
     */
    void Init(ProjectSettingsPtr& projectSettings);

    /**
     * @brief Initializes input manager with player settings (Game runtime usage).
     * @param playerSettings The player settings.
     */
    void Init(TossPlayerSettingsPtr& playerSettings);

    /**
     * @brief Returns true if the specified key is currently being held down.
     * @param key The key code to query.
     * @param checkPlayMode Whether to require play mode to be enabled.
     * @return True if the key is down.
     */
    bool isKeyDown(Key key, bool checkPlayMode = true) const;

    /**
     * @brief Returns true if the specified key was pressed this frame.
     * @param key The key code to query.
     * @param checkPlayMode Whether to require play mode to be enabled.
     * @return True if the key was pressed this frame.
     */
    bool isKeyPressed(Key key, bool checkPlayMode = true) const;

    /**
     * @brief Returns true if the specified key is currently not being held down.
     * @param key The key code to query.
     * @param checkPlayMode Whether to require play mode to be enabled.
     * @return True if the key is up.
     */
    bool isKeyUp(Key key, bool checkPlayMode = true) const;

    /**
     * @brief Checks if the specified mouse button is currently being held down.
     * @param button The mouse button to query.
     * @param checkPlayMode Whether to require play mode to be enabled.
     * @return True if the mouse button is down.
     */
    bool isMouseDown(MouseButton button, bool checkPlayMode = true) const;

    /**
     * @brief Checks if the specified mouse button was pressed this frame.
     * @param button The mouse button to query.
     * @param checkPlayMode Whether to require play mode to be enabled.
     * @return True if the mouse button was pressed this frame.
     */
    bool isMousePressed(MouseButton button, bool checkPlayMode = true) const;

    /**
     * @brief Checks if the specified mouse button is currently not being held down.
     * @param button The mouse button to query.
     * @param checkPlayMode Whether to require play mode to be enabled.
     * @return True if the mouse button is up.
     */
    bool isMouseUp(MouseButton button, bool checkPlayMode = true) const;

    /**
     * @brief Returns current horizontal mouse movement.
     * @param checkPlayMode Whether to require play mode to be enabled.
     * @return Horizontal movement delta (-1.0f to 1.0f).
     */
    float getMouseXAxis(bool checkPlayMode = true) const;

    /**
     * @brief Returns current vertical mouse movement.
     * @param checkPlayMode Whether to require play mode to be enabled.
     * @return Vertical movement delta (-1.0f to 1.0f).
     */
    float getMouseYAxis(bool checkPlayMode = true) const;

    /**
     * @brief Gets the current mouse position.
     * @return Mouse position in screen coordinates.
     */
    static Vector2 getMousePosition();

    /**
     * @brief Gets the current mouse scroll offset.
     * @return Mouse scroll delta.
     */
    static Vector2 getMouseScroll();

    /**
     * @brief Enables or disables play mode.
     * @param enable True to enable, false to disable.
     * @param alsoChangeGameplayMode True to also toggle internal game mode flag.
     */
    void enablePlayMode(bool enable, bool alsoChangeGameplayMode = true);

    /**
     * @brief Toggles the play mode state.
     * @param alsoChangeGameplayMode True to also toggle internal game mode flag.
     */
    void togglePlayMode(bool alsoChangeGameplayMode = true);

    /**
     * @brief Checks if play mode is currently enabled.
     * @return True if play mode is active.
     */
    bool isPlayModeEnabled() const;

    /**
     * @brief Checks if gameplay mode is currently enabled.
     * @return True if game mode is active.
     */
    bool isGameModeEnabled() const;

    /**
     * @brief Sets the screen area for mouse lock when in play mode.
     * @param area The size of the screen area.
     */
    void setScreenArea(const Vector2& area);

    /**
     * @brief Updates input states (should be called every frame).
     */
    void onUpdate();

    /**
     * @brief Post-render update (late frame adjustments).
     */
    void onLateUpdate();

private:
    // Private constructor and destructor
    InputManager() = default;
    ~InputManager() = default;

    bool isInitilized = false; //!< Tracks whether the input manager is initialized.

    // GLFW input callbacks
    static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void char_callback(GLFWwindow* window, unsigned int c);

    // Mouse position and scroll tracking
    static float currentMouseX; //!< Current mouse x position.
    static float currentMouseY; //!< Current mouse y position.
    static double scrollX;      //!< Current scroll x offset.
    static double scrollY;      //!< Current scroll y offset.

    // Input state tracking
    static std::map<Key, bool> currentKeyStates; //!< Current key states.
    static std::map<Key, bool> previousKeyStates; //!< Previous key states.
    static std::map<MouseButton, bool> currentMouseStates; //!< Current mouse button states.
    static std::map<MouseButton, bool> previousMouseStates; //!< Previous mouse button states.

    /**
     * @brief Resets the mouse scroll deltas.
     */
    static void resetMouseScroll();

    // Internal variables
    GLFWwindow* WindowPtr = nullptr; //!< Pointer to the GLFW window context.
    bool m_playEnable = false; //!< Indicates if play mode is active.
    bool m_gameMode = false;   //!< Indicates if game mode is active.
    Vector2 m_oldMousePos{};   //!< Previous mouse position for delta calculations.
    Vector2 m_screenArea;      //!< Screen area used for cursor locking.
    Vector2 m_deltaMouse{};    //!< Delta movement of the mouse this frame.

    static constexpr double MOUSE_MOVEMENT_THRESHOLD = 0.00001; //!< Movement threshold to avoid tiny unintentional drifts.
};