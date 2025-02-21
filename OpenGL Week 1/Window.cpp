/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Window.cpp
Description : Window class is a wrapper of a GLFWwindow to display the rendering from the graphics engine
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "Window.h"
#include "Utils.h"
#include "Game.h"

Window::Window(Game* game) : gameOwner(game)
{
    // Set GLFW window hints
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    //Specify whether to create a forward-compatible context
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create a GLFW window
    m_windowPtr = glfwCreateWindow(m_size.width, m_size.height, "TheoCreates | OpenGL 3D Game", nullptr, nullptr);
    if (!m_windowPtr)
    {
        Debug::LogError("GLFW failed to initialize properly. Terminating program.");
        glfwTerminate();
        return;
    }
    
    // Make the context current before initializing GLEW
    makeCurrentContext(vsync);

    // Initialize GLEW or GLAD here if needed
    if (glewInit() != GLEW_OK)
    {
        Debug::LogError("GLEW failed to initialize properly. Terminating program.");
        glfwTerminate();
        return;
    }

    // Set GLFW user pointer to 'this' for access in callback functions
    glfwSetWindowUserPointer(m_windowPtr, this);

    // Set the window resize callback
    glfwSetWindowSizeCallback(m_windowPtr, [](GLFWwindow* window, int width, int height)
        {
            // Get the user pointer, which is the 'Window' instance
            Window* display = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (display)
            {
                display->onResize(width, height);
            }
        });

    // Show the window
    glfwShowWindow(m_windowPtr);


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_windowPtr, true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

Window::~Window()
{
    glfwMakeContextCurrent(nullptr);
    // Destroy the GLFW window
    glfwDestroyWindow(m_windowPtr);
    // Terminate GLFW
    glfwTerminate();
}

Vector2 Window::getInnerSize()
{
    int width, height;
    glfwGetFramebufferSize(m_windowPtr, &width, &height);
    return Vector2(width, height);
}

GLFWwindow* Window::getWindow()
{
    return m_windowPtr;
}

void Window::makeCurrentContext(bool vsync)
{
    glfwMakeContextCurrent(m_windowPtr);

    glfwSwapInterval(vsync ? 1 : 0);
}

void Window::present()
{
    glfwSwapBuffers(m_windowPtr);
}

void Window::onResize(int _width, int _height)
{
    Debug::Log("Window resized to: " + ToString(_width) + "x" + ToString(_height));
    m_size.width = _width;
    m_size.height = _height;

    gameOwner->onResize(_width, _height);
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(m_windowPtr);
}

