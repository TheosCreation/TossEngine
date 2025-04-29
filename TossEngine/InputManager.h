#pragma once

#include "Utils.h"
#include "Rect.h"
#include "Window.h"
#include <map>

/**
 * @class InputManager
 * @brief Handles inputs from the player/user of the program.
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

    /**
     * @brief Deleted copy constructor to prevent copying.
     */
    InputManager(const InputManager&) = delete;

    /**
     * @brief Deleted assignment operator to prevent assignment.
     */
    void operator=(const InputManager&) = delete;

    void Init(ProjectSettingsPtr& projectSettings);
    void Init(TossPlayerSettingsPtr& playerSettings);

    /**
     * @brief Returns true if key is currently being held down.
     *
     * This method fires every frame that the key is held down, and if
     * checkPlayMode is true it will always return false when play mode
     * is disabled so you wont get accidental pressed events in edit mode.
     *
     * @param key           The key code to query.
     * @param checkPlayMode If true (the default) ignores presses entirely
     *                      when not in play mode.
     * @return              True if the key is held this frame (and play
     *                      mode is on if checking) false otherwise.
     */
    bool isKeyDown(Key key, bool checkPlayMode = true) const;

    /**
     * @brief Returns true on the exact frame that the key transitions from up to down.
     *
     * This method only fires once per press (not continuous), and if
     * checkPlayMode is true it will always return false when play mode
     * is disabled so you wont get accidental pressed events in edit mode.
     *
     * @param key           The key code to query.
     * @param checkPlayMode If true (the default) ignores presses entirely
     *                      when not in play mode.
     * @return              True if the key was pressed this frame (and play
     *                      mode is on if checking) false otherwise.
     */
    bool isKeyPressed(Key key, bool checkPlayMode = true) const;

    /**
     * @brief Checks if the specified key is currently not being held down.
     * @param key The key to check.
     * @param checkPlayMode
     * @return True if the key is up, false otherwise.
     */
    bool isKeyUp(Key key, bool checkPlayMode = true) const;

    /**
     * @brief Checks if the specified mouse button is currently being held down.
     * @param button The mouse button to check.
     * @param checkPlayMode
     * @return True if the mouse button is down, false otherwise.
     */
    bool isMouseDown(MouseButton button, bool checkPlayMode = true) const;

    /**
     * @brief Checks if the specified mouse button was pressed.
     * @param button The mouse button to check.
     * @param checkPlayMode
     * @return True if the mouse button was pressed, false otherwise.
     */
    bool isMousePressed(MouseButton button, bool checkPlayMode = true) const;

    /**
     * @brief Checks if the specified mouse button is currently not being held down.
     * @param button The mouse button to check.
     * @param checkPlayMode
     * @return True if the mouse button is up, false otherwise.
     */
    bool isMouseUp(MouseButton button, bool checkPlayMode = true) const;

    /**
     * @brief Returns the current mouse movement along the x-axis.
     * @return The mouse movement along the x-axis: -1 (left), 1 (right).
     */
    float getMouseXAxis(bool checkPlayMode = true) const;

    /**
     * @brief Returns the current mouse movement along the y-axis.
     * @return The mouse movement along the y-axis: -1 (bottom), 1 (top).
     */
    float getMouseYAxis(bool checkPlayMode = true) const;

    /**
     * @brief Returns the current mouse position as a vector2.
     * @return The current mouse position.
     */
    static Vector2 getMousePosition();

    /**
     * @brief Returns the current mouse scroll as a vector2.
     * @return The current mouse scroll.
     */
    static Vector2 getMouseScroll();

    /**
     * @brief Enables or disables play mode, which hides the cursor and locks it at the center of the screen.
     * @param enable True to enable play mode, false to disable.
     */
    void enablePlayMode(bool enable, bool alsoChangeGameplayMode = true);
    void togglePlayMode(bool alsoChangeGameplayMode = true);

    bool isPlayModeEnabled() const;
    bool isGameModeEnabled() const;
    Rect getViewport();


    /**
     * @brief Sets the screen area the cursor will be locked into when play mode is enabled.
     * @param area The screen area to set.
     */
    void setScreenArea(const Vector2& area);
    void setViewport(float x, float y, float width, float height);
    void disableViewport();

    /**
     * @brief Updates the input states.
     */
    void onUpdate();

    /**
     * @brief Update late post graphics update/render.
     */
    void onLateUpdate();

private:
    /**
     * @brief Private constructor to prevent external instantiation.
     */
    InputManager() = default;

    /**
     * @brief Private destructor.
     */
    ~InputManager() = default;

    bool isInitilized = false;

    /**
     * @brief Static callback functions for handling input events.
     */
    static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void char_callback(GLFWwindow* window, unsigned int c);

    static float currentMouseX; // Current mouse x position
    static float currentMouseY; // Current mouse y position

    static double scrollX; // Current mouse x scroll
    static double scrollY; // Current mouse y scroll

    static std::map<Key, bool> currentKeyStates; // Map of all current key states
    static std::map<Key, bool> previousKeyStates; // Map of all previous frames key states
    static std::map<MouseButton, bool> currentMouseStates; // Map of all current mouse button states
    static std::map<MouseButton, bool> previousMouseStates; // Map of all previous frames mouse button states

    const double MOUSE_MOVEMENT_THRESHOLD = 0.00001f; // Const for the mouse movement threshold

    static void resetMouseScroll(); // Resets scroll x and y to 0

    GLFWwindow* WindowPtr = nullptr; // Pointer to the GLFW window

    bool m_playEnable = false; // Indicates whether play mode is enabled
    bool m_gameMode = false;
    Vector2 m_oldMousePos{}; // Previous mouse position
    Vector2 m_screenArea; // Screen area for cursor locking
    Vector2 m_deltaMouse{}; // Mouse movement delta

    Rect m_viewport = { 0, 0, 1, 1 }; // normalized [0,1] for full window by default
    bool m_viewportEnabled = false;
};