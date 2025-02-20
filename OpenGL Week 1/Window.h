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
#include <glew.h>
#include <glfw3.h>
#include "Utils.h"

class Game;

/**
 * @class Window
 * @brief A wrapper for a GLFWwindow to display the rendering from the graphics engine.
 */
class Window
{
public:
    /**
     * @brief Constructor for the Window class.
     */
    Window(Game* _game);

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

    void onResize(int _width, int _height);

    /**
     * @brief Checks if the window should close.
     * @return True if the window should close, false otherwise.
     */
    bool shouldClose();

private:
    GLFWwindow* m_windowPtr;        // Pointer to the GLFWwindow.
    void* m_context = nullptr;      // Pointer to the OpenGL context.
    bool vsync = false;             // Flag for vertical synchronization.
    Rect m_size = Rect(800, 800);   // The size of the window.
    Game* gameOwner = nullptr;      // The Game Owner of the window.
};