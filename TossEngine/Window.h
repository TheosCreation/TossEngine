/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : Window.h
Description : GLFW-based window wrapper for rendering output and event-driven resizing.
              Integrates with the Resizable system for editor and runtime scaling.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Utils.h"
#include "Resizable.h"

struct GLFWwindow;

/**
 * @class Window
 * @brief Wraps a GLFWwindow instance. Handles window size, vsync, context control, and resize notifications.
 */
class TOSSENGINE_API Window : public Resizable
{
public:
    /**
     * @brief Constructs a new window instance.
     * @param owner Pointer to a Resizable object that receives resize callbacks.
     * @param size Initial window size.
     * @param windowName Title of the window.
     * @param maximized If true, starts the window maximized.
     */
    Window(Resizable* owner, Vector2 size, const std::string& windowName, bool maximized = false);

    /**
     * @brief Destroys the window and releases GLFW resources.
     */
    ~Window();

    /**
     * @brief Gets the inner framebuffer size of the window (in pixels).
     * @return Vector2 representing framebuffer size.
     */
    Vector2 getInnerSize() const;

    /**
     * @brief Gets the underlying GLFWwindow pointer.
     * @return Pointer to the GLFWwindow.
     */
    GLFWwindow* getWindow() const;

    /**
     * @brief Sets the window title.
     * @param windowName New window title.
     */
    void setWindowName(const std::string& windowName);

    /**
     * @brief Changes the object receiving resize and maximize events.
     * @param newOwner Pointer to the new Resizable.
     */
    void setOwner(Resizable* newOwner);

    /**
     * @brief Makes this window the active OpenGL context.
     */
    void makeCurrentContext() const;

    /**
     * @brief Enables or disables VSync.
     * @param _vsync True to enable vsync, false to disable.
     */
    void enableVsync(bool _vsync);

    /**
     * @brief Swaps the front and back buffers to display the rendered frame.
     */
    void present() const;

    /**
     * @brief Flags the window for closing.
     */
    void close() const;

    /**
     * @brief Handles logic when the window is resized.
     * @param size New size.
     */
    void onResize(Vector2 size) override;

    /**
     * @brief Handles logic when the window is maximized.
     * @param maximized 1 if maximized, 0 otherwise.
     */
    void onMaximize(int maximized) override;

    /**
     * @brief Checks if the window should close (e.g., user pressed close button).
     * @return True if window should close.
     */
    bool shouldClose() const;

private:
    GLFWwindow* m_windowPtr = nullptr;       //!< Pointer to the GLFWwindow.
    bool vsync = false;                      //!< Whether VSync is currently enabled.
    Resizable* m_resizableOwner = nullptr;   //!< Object to notify on resize/maximize.
    std::string m_windowName;                //!< Window title.
};