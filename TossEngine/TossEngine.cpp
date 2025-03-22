#include "TossEngine.h"
#include "ResourceManager.h"
#include <glew.h>
#include <glfw3.h>
#include <windows.h>
#include "tinyfiledialogs.h"

void TossEngine::Init()
{
    if (isInitilized) return;

    //init GLFW ver 4.6
    if (!glfwInit())
    {
        Debug::LogError("GLFW failed to initialize properly. Terminating program.");
        return;
    }
    LoadScripts();
    isInitilized = true;
}

void TossEngine::LoadScripts()
{
    HMODULE scriptsDll = LoadLibrary(L"Scripts.dll");
    if (scriptsDll) {
        typedef void (*RegisterComponentsFunc)();
        RegisterComponentsFunc registerFunc = (RegisterComponentsFunc)GetProcAddress(scriptsDll, "RegisterComponents");
        if (registerFunc) {
            registerFunc();
        }
    }
}

void TossEngine::LoadGenericResources()
{
    auto& resourceManager = ResourceManager::GetInstance();
    resourceManager.createMeshFromFile("Resources/Meshes/sphere.obj");
    resourceManager.createMeshFromFile("Resources/Meshes/cube.obj");
}

void TossEngine::TryCreateWindow(Resizable* owner, Vector2 size, const string& windowName)
{
    if (m_window == nullptr)
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

float TossEngine::GetTime()
{
    return static_cast<float>(glfwGetTime());
}

std::string TossEngine::openFileDialog(const std::string& filter)
{
    const char* filterArray[] = { filter.c_str() };
    const char* filePath = tinyfd_openFileDialog("Select File", "", 1, filterArray, "Select a File", 0);

    return filePath ? std::string(filePath) : "";
}