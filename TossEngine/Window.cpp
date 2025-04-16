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
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glew.h>
#include <glfw3.h>
#include <ImGuizmo.h>

Window::Window(Resizable* owner, Vector2 size, const string& windowName, bool maximized)
{
    m_size = size;
    m_resizableOwner = owner;
    m_windowName = windowName;
    m_maximized = maximized;

    // Set GLFW window hints
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    //Specify whether to create a forward-compatible context
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create a GLFW window
    m_windowPtr = glfwCreateWindow(m_size.x, m_size.y, windowName.c_str(), nullptr, nullptr);
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
                display->onResize(Vector2(width, height));
            }
        });

    glfwSetWindowMaximizeCallback(m_windowPtr, [](GLFWwindow* window, int maximized) {
        Window* display = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (display) {
            display->onMaximize(maximized);
        }
        });

    // Show the window
    glfwShowWindow(m_windowPtr); 
    
    if (m_maximized) {
        glfwMaximizeWindow(m_windowPtr);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_windowPtr, true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

Window::~Window()
{
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();  // Cleanup OpenGL bindings
    ImGui_ImplGlfw_Shutdown();     // Cleanup GLFW bindings
    ImGui::DestroyContext();       // Destroy ImGui context

    glfwMakeContextCurrent(nullptr);
    // Destroy the GLFW window
    glfwDestroyWindow(m_windowPtr);
    // Terminate GLFW
    //glfwTerminate();
}

Vector2 Window::getInnerSize() const
{
    int width, height;
    glfwGetFramebufferSize(m_windowPtr, &width, &height);
    return {static_cast<float>(width), static_cast<float>(height)};
}

GLFWwindow* Window::getWindow() const
{
    return m_windowPtr;
}

void Window::setWindowName(const string& windowName)
{
    m_windowName = windowName;

    if (m_windowPtr) {
        glfwSetWindowTitle(m_windowPtr, windowName.c_str()); // Update GLFW window title
    }
}

void Window::setOwner(Resizable* newOwner)
{
    m_resizableOwner = newOwner;
}

void Window::makeCurrentContext(bool vSync) const
{
    glfwMakeContextCurrent(m_windowPtr);

    glfwSwapInterval(vSync ? 1 : 0);
}

void Window::present() const
{
    glfwSwapBuffers(m_windowPtr);
}

void Window::close() const
{
    if(m_windowPtr) {
        glfwSetWindowShouldClose(m_windowPtr, GLFW_TRUE);
    }
}

void Window::onResize(Vector2 size)
{
    Resizable::onResize(size);
    //Debug::Log("Window resized to: " + ToString(size.x) + "x" + ToString(size.y));

    if (size.Length() <= 10.0f) return;
    m_resizableOwner->onResize(size);
}

void Window::onMaximize(int maximized)
{
    Resizable::onMaximize(maximized);
    m_resizableOwner->onMaximize(maximized);
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(m_windowPtr);
}

