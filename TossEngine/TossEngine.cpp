#include "TossEngine.h"
#include "ResourceManager.h"
#include <glew.h>
#include <glfw3.h>
#include <windows.h>
#include "tinyfiledialogs.h"
#include "ScriptLoader.h"
#include "TossPlayerSettings.h"

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

    LayerManager& layerManager = LayerManager::GetInstance();
    json data = JsonUtility::OpenJsonFile("LayerManager.json");
    if (!data.empty())
    {
        layerManager.deserialize(data);
    }
}

void TossEngine::LoadGenericResources()
{
    auto& resourceManager = ResourceManager::GetInstance();

    // Meshes
    json meshMeta = json::object();
    meshMeta["m_path"] = "Internal/Meshes/sphere.obj";
    resourceManager.createInternalResource("Mesh", "Sphere", meshMeta);
    

    json cubeMeta = json::object();
    cubeMeta["m_path"] = "Internal/Meshes/cube.obj";
    resourceManager.createInternalResource("Mesh", "Cube", cubeMeta);

    //Shaders
    json skyBoxShaderMeta = json::object();
    skyBoxShaderMeta["m_path"] = "Internal/Shaders/SkyboxShader.shaderprog";
    resourceManager.createInternalResource("Shader", "SkyBoxShader", skyBoxShaderMeta);
    
    json geometryPassMeshShaderMeta = json::object();
    geometryPassMeshShaderMeta["m_path"] = "Internal/Shaders/GeometryPassMeshShader.shaderprog";
    resourceManager.createInternalResource("Shader", "GeometryPassMeshShader", geometryPassMeshShaderMeta);
    
    {
        json shaderMeta = json::object();
        shaderMeta["m_path"] = "Internal/Shaders/GeometryPassInstancedMeshShader.shaderprog";
        resourceManager.createInternalResource("Shader", "GeometryPassInstancedMeshShader", shaderMeta);
    }
    {
        json shaderMeta = json::object();
        shaderMeta["m_path"] = "Internal/Shaders/Image.shaderprog";
        resourceManager.createInternalResource("Shader", "Image", shaderMeta);
    }
    {
        json shaderMeta = json::object();
        shaderMeta["m_path"] = "Internal/Shaders/MeshShader.shaderprog";
        resourceManager.createInternalResource("Shader", "MeshShader", shaderMeta);
    }
    {
        json shaderMeta = json::object();
        shaderMeta["m_path"] = "Internal/Shaders/InstancedMeshShader.shaderprog";
        resourceManager.createInternalResource("Shader", "InstancedMeshShader", shaderMeta);
    }
    {
        json shaderMeta = json::object();
        shaderMeta["m_path"] = "Internal/Shaders/Text.shaderprog";
        resourceManager.createInternalResource("Shader", "Text", shaderMeta);
    }
    {
        json shaderMeta = json::object();
        shaderMeta["m_path"] = "Internal/Shaders/ShadowShaderInstancedShader.shaderprog";
        resourceManager.createInternalResource("Shader", "ShadowShaderInstancedShader", shaderMeta);
    }
    {
        json shaderMeta = json::object();
        shaderMeta["m_path"] = "Internal/Shaders/ShadowShader.shaderprog";
        resourceManager.createInternalResource("Shader", "ShadowShader", shaderMeta);
    }
    {
        json shaderMeta = json::object();
        shaderMeta["m_path"] = "Internal/Shaders/SSQLightingShader.shaderprog";
        resourceManager.createInternalResource("Shader", "SSQLightingShader", shaderMeta);
    }
    {
        json shaderMeta = json::object();
        shaderMeta["m_path"] = "Internal/Shaders/DebugShader.shaderprog";
        resourceManager.createInternalResource("Shader", "DebugShader", shaderMeta);
    }
    {
        json shaderMeta = json::object();
        shaderMeta["m_path"] = "Internal/Shaders/PostProcessPlainShader.shaderprog";
        resourceManager.createInternalResource("Shader", "PostProcessPlainShader", shaderMeta);
    }
    {
        json shaderMeta = json::object();
        shaderMeta["m_path"] = "Internal/Shaders/PostProcessOldFilmShader.shaderprog";
        resourceManager.createInternalResource("Shader", "PostProcessOldFilmShader", shaderMeta);
    }
    {
        json shaderMeta = json::object();
        shaderMeta["m_path"] = "Internal/Shaders/PostProcessCRTShader.shaderprog";
        resourceManager.createInternalResource("Shader", "PostProcessCRTShader", shaderMeta);
    }
    {
        json shaderMeta = json::object();
        shaderMeta["m_path"] = "Internal/Shaders/PostProcessInvertedShader.shaderprog";
        resourceManager.createInternalResource("Shader", "PostProcessInvertedShader", shaderMeta);
    }
    {
        json shaderMeta = json::object();
        shaderMeta["m_path"] = "Internal/Shaders/PostProcessLumGreyscale.shaderprog";
        resourceManager.createInternalResource("Shader", "PostProcessLumGreyscale", shaderMeta);
    }
    {
        json shaderMeta = json::object();
        shaderMeta["m_path"] = "Internal/Shaders/ParticleSystem.shaderprog";
        resourceManager.createInternalResource("Shader", "ParticleSystem", shaderMeta);
    }

    //Materials
    {
        json materialMeta = json::object();
        materialMeta["m_path"] = "Internal/Materials/SkyboxMaterial.mat";
        resourceManager.createInternalResource("Material", "SkyboxMaterial", materialMeta);
    }
    {
        json materialMeta = json::object();
        materialMeta["m_path"] = "Internal/Materials/ImageMaterial.mat";
        resourceManager.createInternalResource("Material", "ImageMaterial", materialMeta);
    }
    {
        json materialMeta = json::object();
        materialMeta["m_path"] = "Internal/Materials/TextMaterial.mat";
        resourceManager.createInternalResource("Material", "TextMaterial", materialMeta);
    }
    {
        json materialMeta = json::object();
        materialMeta["m_path"] = "Internal/Materials/PostProcessSSRQMaterial.mat";
        resourceManager.createInternalResource("Material", "PostProcessSSRQMaterial", materialMeta);
    }
    {
        json materialMeta = json::object();
        materialMeta["m_path"] = "Internal/Materials/DeferredSSRQMaterial.mat";
        resourceManager.createInternalResource("Material", "DeferredSSRQMaterial", materialMeta);
    }

    // Cubemaps
    {
        json cubemapMeta = json::object();
        cubemapMeta["m_path"] = "Internal/Cubemaps/RedEclipse/RedEclipse.cubemap";
        resourceManager.createInternalResource("TextureCubeMap", "RedEclipse", cubemapMeta);
    }

    // Fonts
    {
        json fontMeta = json::object();
        fontMeta["m_path"] = "Internal/Fonts/VCR_OSD_MONO.ttf";
        resourceManager.createInternalResource("Font", "VCRFont", fontMeta);
    }
    
    // Physics materials
    {
        json fontMeta = json::object();
        fontMeta["m_path"] = "Internal/PhysicsMaterials/DefaultPhysicsMaterial.physicsmaterial";
        resourceManager.createInternalResource("PhysicsMaterial", "DefaultPhysicsMaterial", fontMeta);
    }
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


void TossEngine::Quit()
{
    if (!isDebugMode)
    {
        m_window->close();
    }
    else
    {
        Debug::Log("Quit Called but did nothing in editor mode");
    }
}

void TossEngine::SetDebugMode(bool enabled)
{
    isDebugMode = enabled;
}

void TossEngine::OpenSceneInternal(std::shared_ptr<Scene> _scene, bool callStartMethods)
{
    // if there is a current scene, call onDestroy
    if (m_currentScene != nullptr)
    {
        m_currentScene->onQuit();
        m_currentScene = nullptr;
    }


    // set the current scene to the new scene
    m_currentScene = std::move(_scene);
    m_currentScene->onCreate();

    if (callStartMethods)
    {
        m_currentScene->onStart();
    }
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

Window* TossEngine::GetWindow() const
{
    return m_window.get();
}

ScenePtr TossEngine::getCurrentScene() const
{
    return m_currentScene;
}

void TossEngine::PollEvents()
{
    glfwPollEvents();
}

void TossEngine::CleanUp()
{
    Physics::GetInstance().CleanUp();
    ComponentRegistry::GetInstance().CleanUp();
    AudioEngine::GetInstance().CleanUp();

    running = false;
    coroutineCondition.notify_all();
    if (coroutineThread.joinable())
        coroutineThread.join();
    m_scriptLoader->unloadDLL();

    glfwTerminate();
}

void TossEngine::UnLoadScripts() const
{
    m_scriptLoader->unloadDLL();
}

void TossEngine::LoadScripts() const
{
    m_scriptLoader->loadDLL();
}

void TossEngine::ReloadScripts() const
{
    m_scriptLoader->reloadDLL();
}

void TossEngine::onUpdateInternal()
{
    for(auto& pair : m_scenesToOpen)
    {
        OpenSceneInternal(pair.first, pair.second);
    }
    m_scenesToOpen.clear();
}

void TossEngine::OpenScene(shared_ptr<Scene> _scene, bool callStartMethods)
{
    m_scenesToOpen.emplace(_scene, callStartMethods);
}

void TossEngine::OpenScene(const string& sceneName)
{
    if (!m_currentPlayerSettings)
    {
        Debug::LogError("Player Settings has not been set in the Toss Engine, cannot open Scene");
        return;
    }

    std::string foundPath;
    for (const auto& path : m_currentPlayerSettings->selectedSceneFilePaths)
    {
        if (path.find(sceneName) != std::string::npos)
        {
            foundPath = path;
            break;
        }
    }

    if (foundPath.empty())
    {
        Debug::LogError("Scene not found: " + sceneName);
        return;
    }

    auto scene = std::make_shared<Scene>(foundPath);
    m_scenesToOpen.emplace(scene, true);
}

void TossEngine::SetPlayerSettings(TossPlayerSettings* playerSettings)
{
    m_currentPlayerSettings = playerSettings;
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

string TossEngine::openFolderDialog()
{
    const char* folderPath = tinyfd_selectFolderDialog("Select Folder", "");

    return folderPath ? std::string(folderPath) : "";
}