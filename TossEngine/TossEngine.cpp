#include "TossEngine.h"
#include "ResourceManager.h"
#include <glew.h>
#include <glfw3.h>
#include <windows.h>
#include "tinyfiledialogs.h"
#include "ScriptLoader.h"

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

    m_scriptLoader = new ScriptLoader();
}

void TossEngine::LoadScripts()
{
    HMODULE scriptsDll = LoadLibrary(L"C++Scripts.dll");
    if (scriptsDll) 
    {
    }
    else
    {
        Debug::Log("No Scripts dll found");
    }
}

void TossEngine::CompileScriptsProject() 
{

}

void TossEngine::LoadGenericResources()
{
    auto& resourceManager = ResourceManager::GetInstance();
    resourceManager.createMeshFromFile("Resources/Meshes/sphere.obj");
    resourceManager.createMeshFromFile("Resources/Meshes/cube.obj");
}

void TossEngine::TryCreateWindow(Resizable* owner, Vector2 size, const string& windowName, bool maximized)
{
    if (m_window == nullptr)
    {
        // Create the window if there isnt one already
        m_window = std::make_unique<Window>(owner, size, windowName, maximized);
    }
}

bool TossEngine::IsDebugMode()
{
    return isDebugMode;
}

void TossEngine::SetDebugMode(bool enabled)
{
    isDebugMode = enabled;
}

ScriptLoader* TossEngine::getScriptLoader()
{
    return m_scriptLoader;
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
    ComponentRegistry::GetInstance().CleanUp();
    isInitilized = false; 

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