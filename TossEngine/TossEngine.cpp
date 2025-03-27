#include "TossEngine.h"
#include "ResourceManager.h"
#include <glew.h>
#include <glfw3.h>
#include <windows.h>
#include "tinyfiledialogs.h"
#include "ScriptLoader.h"

void TossEngine::Init()
{
    if (running) return;

    //init GLFW ver 4.6
    if (!glfwInit())
    {
        Debug::LogError("GLFW failed to initialize properly. Terminating program.", false);
        return;
    }

    running = true;

    m_scriptLoader = new ScriptLoader();
    coroutineThread = std::thread(&TossEngine::CoroutineRunner, this);

    Physics::GetInstance().Init();
}

void TossEngine::LoadGenericResources()
{
   // auto& resourceManager = ResourceManager::GetInstance();
   // resourceManager.createMeshFromFile("Resources/Meshes/sphere.obj");
   // resourceManager.createMeshFromFile("Resources/Meshes/cube.obj");
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

void TossEngine::CoroutineRunner()
{
    while (running)
    {
        CoroutineTask currentTask(nullptr);
        {
            std::unique_lock<std::mutex> lock(coroutineMutex);
            coroutineCondition.wait(lock, [this]() {
                return !coroutineQueue.empty() || !running;
                });

            if (!running && coroutineQueue.empty())
                return;

            currentTask = std::move(coroutineQueue.front());
            coroutineQueue.pop();
        }

        while (currentTask.Resume())
        {
            // Simulate waiting or yielding, depending on your coroutine logic
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        // Coroutine finished execution
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
    Physics::GetInstance().CleanUp();
    ComponentRegistry::GetInstance().CleanUp();
    running = false;
    coroutineCondition.notify_all();
    if (coroutineThread.joinable())
        coroutineThread.join();
    m_scriptLoader->unloadDLL();

    glfwTerminate();
}

void TossEngine::UnLoadScripts()
{
    m_scriptLoader->unloadDLL();
}

void TossEngine::LoadScripts()
{
    m_scriptLoader->loadDLL();
}

void TossEngine::ReloadScripts()
{
    m_scriptLoader->reloadDLL();
}

float TossEngine::GetTime()
{
    return static_cast<float>(glfwGetTime());
}

std::shared_ptr<bool> TossEngine::StartCoroutine(CoroutineTask&& coroutine)
{
    auto doneFlag = std::make_shared<bool>(false);

    // Capture the flag in the coroutine and set it on completion
    coroutine.SetOnComplete([doneFlag]() {
        *doneFlag = true;
        });

    {
        std::lock_guard<std::mutex> lock(coroutineMutex);
        coroutineQueue.emplace(std::move(coroutine));
        coroutineCondition.notify_one();
    }

    return doneFlag;
}

std::string TossEngine::openFileDialog(const std::string& filter)
{
    const char* filterArray[] = { filter.c_str() };
    const char* filePath = tinyfd_openFileDialog("Select File", "", 1, filterArray, "Select a File", 0);

    return filePath ? std::string(filePath) : "";
}