/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Game.cpp
Description : Game class that controls the order the graphics engine and internal systems performs tasks
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "Game.h"
#include <filesystem>
#include "TossEngine.h"
#include "Window.h"
#include "VertexArrayObject.h"
#include "Shader.h"
#include "GameObjectManager.h"
#include "GraphicsGameObject.h"
#include "Player.h"
#include "TextureCubeMap.h"
#include "Camera.h"
#include "SkyBoxGameObject.h"
#include "Scene.h"
#include "Framebuffer.h"
#include "GeometryBuffer.h"
#include "ProjectSettings.h"
#include "AudioEngine.h"
#include <imgui.h>

Game::Game(ProjectSettingsPtr& projectSettings)
{
    auto& tossEngine = TossEngine::GetInstance();
    tossEngine.Init();
    Vector2 windowSize = Vector2(800, 800);
    tossEngine.CreateWindowOrChangeOwner(this, windowSize, "TossEditor");

    MonoIntegration::InitializeMono();

    initRandomSeed();
    GeometryBuffer::GetInstance().Init(windowSize);

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.Init(projectSettings);
    graphicsEngine.setViewport(windowSize);
    graphicsEngine.setDepthFunc(DepthType::Less);
    graphicsEngine.setBlendFunc(BlendType::SrcAlpha, BlendType::OneMinusSrcAlpha);
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
    graphicsEngine.setScissorSize(Rect(200, 200, 400, 300));
    graphicsEngine.setMultiSampling(true);

    auto& inputManager = InputManager::GetInstance();
    inputManager.Init(projectSettings);
    inputManager.setScreenArea(windowSize);

    LightManager::GetInstance().Init();
    auto& audioEngine = AudioEngine::GetInstance();
    audioEngine.Init();
}

Game::~Game()
{
}

void Game::onCreate()
{
    auto scene = std::make_shared<Scene>();
    SetScene(scene);
}

void Game::onCreateLate()
{
    m_currentScene->onCreateLate();
}

void Game::onUpdateInternal()
{
    auto& tossEngine = TossEngine::GetInstance();
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    auto& inputManager = InputManager::GetInstance();
    auto& audioEngine = AudioEngine::GetInstance();
    inputManager.onUpdate();

    // delta time
    m_currentTime = tossEngine.GetCurrentTime();
    float deltaTime = m_currentTime - m_previousTime;
    m_previousTime = m_currentTime;

    // Accumulate time
    m_accumulatedTime += deltaTime;

    // Perform updates
    while (m_accumulatedTime >= m_fixedTimeStep)
    {
        float fixedDeltaTime = m_currentTime - m_previousFixedUpdateTime;
        m_previousFixedUpdateTime = m_currentTime;
        m_currentScene->onFixedUpdate(fixedDeltaTime);
        m_accumulatedTime -= m_fixedTimeStep;
    }

    m_currentScene->onUpdate(deltaTime);

    m_currentScene->onLateUpdate(deltaTime);

    audioEngine.Update();

    inputManager.onLateUpdate();

    double RenderTime_Begin = (double)glfwGetTime();

    graphicsEngine.clear(glm::vec4(0, 0, 0, 1)); //clear the existing stuff first is a must
    
    //ImGui_ImplOpenGL3_NewFrame();
    //ImGui_ImplGlfw_NewFrame();
    //ImGui::NewFrame();

    m_currentScene->onGraphicsUpdate(); //Render the scene

    //ImGui::Begin("Settings menu lol");
    //ImGui::Text("These are your options");

    //static const char* items[]{ "Deferred Rendering","Forward" }; static int Selecteditem = (int)m_projectSettings->renderingPath;
    //if (ImGui::Combo("Rendering Path", &Selecteditem, items, IM_ARRAYSIZE(items)))
    //{
    //    RenderingPath selectedPath = static_cast<RenderingPath>(Selecteditem);
    //    Debug::Log("Rendering Path changed to option: " + ToString(selectedPath));
    //    graphicsEngine.setRenderingPath(selectedPath);
    //    m_projectSettings->renderingPath = selectedPath;
    //}
    
    if (ImGui::Button("Play"))
    {
       // m_isRunning = true;
    }
    //
    //if (ImGui::Button("Stop"))
    //{
    //    m_isRunning = false;
    //}
    //
    //ImGui::End();
    //
    //ImGui::Render();
    //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    double RenderTime_End = (double)glfwGetTime();

    // Render to window
    tossEngine.GetWindow()->present();
}

void Game::onQuit()
{
    m_currentScene->onQuit();

    MonoIntegration::ShutdownMono();
}

void Game::run()
{
	onCreate();
	onCreateLate();

    auto& tossEngine = TossEngine::GetInstance();
    //run funcs while window open
    while (tossEngine.GetWindow()->shouldClose() == false)
    {
        auto& tossEngine = TossEngine::GetInstance();
        tossEngine.PollEvents();
        onUpdateInternal();
    }

    onQuit();
}

void Game::onResize(Vector2 size)
{
    Resizable::onResize(size);

    GraphicsEngine::GetInstance().setViewport(Vector2(size.x, size.y));
    m_currentScene->onResize(size);
    GeometryBuffer::GetInstance().Resize(size);
}

void Game::SetScene(shared_ptr<Scene> _scene)
{
    // if there is a current scene, call onQuit
    if (m_currentScene != nullptr)
    {
        m_currentScene->onQuit();
        LightManager::GetInstance().reset(); 
        ResourceManager::GetInstance().ClearInstancesFromMeshes();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    // set the current scene to the new scene
    m_currentScene = std::move(_scene);
    m_currentScene->onCreate();
    m_currentScene->onCreateLate();
}