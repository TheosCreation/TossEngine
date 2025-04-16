/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Window.h
Description : Window class is a wrapper of a GLFWwindow to display the rendering from the graphics engine
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Utils.h"
#include "Resizable.h"

struct GLFWwindow;

/**
 * @class Window
 * @brief A wrapper for a GLFWwindow to display the rendering from the graphics engine.
 */
class TOSSENGINE_API Window : public Resizable
{
public:
    /**
     * @brief Constructor for the Window class.
     */
    Window(Resizable* owner, Vector2 size, const string& windowName, bool maximized = false);

    /**
     * @brief Destructor for the Window class.
     */
    ~Window();

    /**
     * @brief Gets the inner size of the window.
     * @return A Vector2 representing the inner size of the window.
     */
    Vector2 getInnerSize() const;

    /**
     * @brief Gets the GLFWwindow pointer.
     * @return A pointer to the GLFWwindow.
     */
    GLFWwindow* getWindow() const;
    void setWindowName(const string& windowName);
    void setOwner(Resizable* newOwner);

    /**
     * @brief Makes the window the current OpenGL context.
     * @param _vsync Enables or disables vertical synchronization.
     */
    void makeCurrentContext(bool _vsync) const;

    /**
     * @brief Presents the rendered content to the window.
     */
    void present() const;
    void close() const;

    void onResize(Vector2 size) override;
    void onMaximize(int maximized) override;

    /**
     * @brief Checks if the window should close.
     * @return True if the window should close, false otherwise.
     */
    bool shouldClose() const;

private:
    GLFWwindow* m_windowPtr;        // Pointer to the GLFWwindow.
    bool vsync = false;             // Flag for vertical synchronization.
    Resizable* m_resizableOwner = nullptr;      // The Owner of the window
    string m_windowName;
};