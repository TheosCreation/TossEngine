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
#include <ImGuizmo.h>

#ifdef __PROSPERO__
#else
#include <glew.h>
#include <glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

Window::Window(Resizable* owner, Vector2 size, const string& windowName, bool maximized)
{
    m_size = size;
    m_resizableOwner = owner;
    m_windowName = windowName;
    m_maximized = maximized;

#ifdef __PROSPERO__
#else
    // Set GLFW window hints
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    //Specify whether to create a forward-compatible context
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create a GLFW window
    m_nativeHandle = glfwCreateWindow(static_cast<int>(m_size.x), static_cast<int>(m_size.y), windowName.c_str(), nullptr, nullptr);
    if (!m_nativeHandle)
    {
        Debug::LogError("GLFW failed to initialize properly. Terminating program.");
        glfwTerminate();
        return;
    }

    // Make the context current before initializing GLEW
    enableVsync(vsync);
    makeCurrentContext();

    // Initialize GLEW or GLAD here if needed
    if (glewInit() != GLEW_OK)
    {
        Debug::LogError("GLEW failed to initialize properly. Terminating program.");
        glfwTerminate();
        return;
    }

    // Set GLFW user pointer to 'this' for access in callback functions
    glfwSetWindowUserPointer(static_cast<GLFWwindow*>(m_nativeHandle), this);

    // Set the window resize callback
    glfwSetWindowSizeCallback(static_cast<GLFWwindow*>(m_nativeHandle), [](GLFWwindow* window, int width, int height)
        {
            // Get the user pointer, which is the 'Window' instance
            if (Window* display = static_cast<Window*>(glfwGetWindowUserPointer(window)))
            {
                display->onResize(Vector2(static_cast<float>(width), static_cast<float>(height)));
            }
        });

    glfwSetWindowMaximizeCallback(static_cast<GLFWwindow*>(m_nativeHandle), [](GLFWwindow* window, int maximized) {
        if (Window* display = static_cast<Window*>(glfwGetWindowUserPointer(window))) {
            display->onMaximize(maximized);
        }
        });

    // Show the window
    glfwShowWindow(static_cast<GLFWwindow*>(m_nativeHandle));
    
    if (m_maximized) {
        glfwMaximizeWindow(static_cast<GLFWwindow*>(m_nativeHandle));
    }
#endif

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

#ifdef __PROSPERO__
#else
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(m_nativeHandle), true);
    ImGui_ImplOpenGL3_Init("#version 460");
#endif
}

Window::~Window()
{
    // Cleanup ImGui
#ifdef __PROSPERO__
#else
    ImGui_ImplOpenGL3_Shutdown();  // Cleanup OpenGL bindings
    ImGui_ImplGlfw_Shutdown();     // Cleanup GLFW bindings
#endif

    ImGui::DestroyContext();       // Destroy ImGui context

#ifdef __PROSPERO__
#else
    glfwMakeContextCurrent(nullptr);
    // Destroy the GLFW window
    glfwDestroyWindow(static_cast<GLFWwindow*>(m_nativeHandle));
    // Terminate GLFW
    //glfwTerminate();
#endif
}

Vector2 Window::getInnerSize() const
{
    int width, height;
#ifdef __PROSPERO__
#else
    glfwGetFramebufferSize(static_cast<GLFWwindow*>(m_nativeHandle), &width, &height);
#endif
    return {static_cast<float>(width), static_cast<float>(height)};
}

void* Window::getNativeHandle() const
{
    return m_nativeHandle;
}

void Window::setWindowName(const string& windowName)
{
    m_windowName = windowName;

    if (m_nativeHandle) {
#ifdef __PROSPERO__
#else
        glfwSetWindowTitle(static_cast<GLFWwindow*>(m_nativeHandle), windowName.c_str()); // Update GLFW window title
#endif
    }
}

void Window::setOwner(Resizable* newOwner)
{
    m_resizableOwner = newOwner;
}

void Window::makeCurrentContext() const
{
#ifdef __PROSPERO__
#else
    glfwMakeContextCurrent(static_cast<GLFWwindow*>(m_nativeHandle));
#endif
}

void Window::enableVsync(bool _vsync)
{
    vsync = _vsync;
#ifdef __PROSPERO__
#else
    glfwSwapInterval(_vsync ? 1 : 0);
#endif
}

void Window::present() const
{
#ifdef __PROSPERO__
#else
    glfwSwapBuffers(static_cast<GLFWwindow*>(m_nativeHandle));
#endif
}

void Window::close() const
{
    if(m_nativeHandle) {
#ifdef __PROSPERO__
#else
        glfwSetWindowShouldClose(static_cast<GLFWwindow*>(m_nativeHandle), GLFW_TRUE);
#endif
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
#ifdef __PROSPERO__
    return false;
#else
    return glfwWindowShouldClose(static_cast<GLFWwindow*>(m_nativeHandle));
#endif
}

