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
#include <memory>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glew.h>
#include <glfw3.h>
#include "Utils.h"
#include "Resizable.h"

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
    Window(Resizable* owner, Vector2 size, const string& windowName);

    /**
     * @brief Destructor for the Window class.
     */
    ~Window();

    /**
     * @brief Gets the inner size of the window.
     * @return A Vector2 representing the inner size of the window.
     */
    Vector2 getInnerSize();

    /**
     * @brief Gets the GLFWwindow pointer.
     * @return A pointer to the GLFWwindow.
     */
    GLFWwindow* getWindow();

    /**
     * @brief Makes the window the current OpenGL context.
     * @param _vsync Enables or disables vertical synchronization.
     */
    void makeCurrentContext(bool _vsync);

    /**
     * @brief Presents the rendered content to the window.
     */
    void present();

    void onResize(Vector2 size) override;

    /**
     * @brief Checks if the window should close.
     * @return True if the window should close, false otherwise.
     */
    bool shouldClose();

private:
    GLFWwindow* m_windowPtr;        // Pointer to the GLFWwindow.
    void* m_context = nullptr;      // Pointer to the OpenGL context.
    bool vsync = false;             // Flag for vertical synchronization.
    Resizable* resizableOwner = nullptr;      // The Owner of the window
    string m_windowName = "";      //
};