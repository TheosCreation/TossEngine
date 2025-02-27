#include "TossEngine.h"
#include "Window.h"
#include <glew.h>
#include <glfw3.h>

void TossEngine::Init()
{
    if (isInitilized) return;

    //init GLFW ver 4.6
    if (!glfwInit())
    {
        Debug::LogError("GLFW failed to initialize properly. Terminating program.");
        return;
    }

    isInitilized = true;
}

void TossEngine::CreateWindowOrChangeOwner(Resizable* owner, Vector2 size, const string& windowName)
{
    if (m_window != nullptr)
    {
        // Change the owner to the new owner
        m_window->setOwner(owner);
        m_window->onResize(size);
        m_window->setWindowName(windowName);
    }
    else
    {
        // Create the window if there isnt one already
        m_window = std::make_unique<Window>(owner, size, windowName);
    }
}

Window* TossEngine::GetWindow()
{
    return m_window.get();
}

void TossEngine::PollEvents()
{
    glfwPollEvents();
}

void TossEngine::CleanUp()
{
    glfwTerminate();
}

float TossEngine::GetCurrentTime()
{
    return static_cast<float>(glfwGetTime());
}
