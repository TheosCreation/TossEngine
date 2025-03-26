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
#include "Player.h"
#include "TextureCubeMap.h"
#include "Camera.h"
#include "Scene.h"
#include "Framebuffer.h"
#include "GeometryBuffer.h"
#include "ProjectSettings.h"
#include "TossPlayerSettings.h"
#include "AudioEngine.h"
#include <imgui.h>
#include <ImGuizmo.h>

Game::Game(ProjectSettingsPtr& projectSettings)
{
    auto& tossEngine = TossEngine::GetInstance();
    tossEngine.Init();
    Vector2 windowSize = Vector2(800, 800);
    tossEngine.TryCreateWindow(this, windowSize, "Game");

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

    AudioEngine::GetInstance().Init();
}

Game::Game(TossPlayerSettingsPtr& playerSettings)
{
    auto& tossEngine = TossEngine::GetInstance();
    tossEngine.Init();
    Vector2 windowSize = Vector2(800, 800);
    tossEngine.TryCreateWindow(this, windowSize, "Game");

    initRandomSeed();
    GeometryBuffer::GetInstance().Init(windowSize);

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.Init(playerSettings);
    graphicsEngine.setViewport(windowSize);
    graphicsEngine.setDepthFunc(DepthType::Less);
    graphicsEngine.setBlendFunc(BlendType::SrcAlpha, BlendType::OneMinusSrcAlpha);
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
    graphicsEngine.setScissorSize(Rect(200, 200, 400, 300));
    graphicsEngine.setMultiSampling(true);

    auto& inputManager = InputManager::GetInstance();
    inputManager.Init(playerSettings);
    inputManager.setScreenArea(windowSize);

    AudioEngine::GetInstance().Init();
}

Game::~Game()
{
}

void Game::onCreate()
{
    //auto scene = std::make_shared<Scene>("Scenes/Scene1.json");
    //SetScene(scene);
}

void Game::onCreateLate()
{
    m_currentScene->onCreateLate();
}

void Game::onUpdateInternal()
{
    auto& tossEngine = TossEngine::GetInstance();

    // delta time
    m_currentTime = tossEngine.GetTime();
    float deltaTime = m_currentTime - m_previousTime;
    m_previousTime = m_currentTime;

    // Accumulate time
    m_accumulatedTime += deltaTime;

    // Perform updates
    while (m_accumulatedTime >= m_fixedTimeStep)
    {
        float fixedDeltaTime = m_currentTime - m_previousFixedUpdateTime;
        m_previousFixedUpdateTime = m_currentTime;
        onFixedUpdate(fixedDeltaTime);
        m_accumulatedTime -= m_fixedTimeStep;
    }
    onUpdate(deltaTime);
    onLateUpdate(deltaTime);

    double RenderTime_Begin = (double)glfwGetTime();

    onGraphicsUpdate();

    double RenderTime_End = (double)glfwGetTime();

    // Render to window
    tossEngine.GetWindow()->present();
}

void Game::onQuit()
{
    m_currentScene->onQuit();
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
    if (m_currentScene != nullptr)
    {
        m_currentScene->onResize(size);
    }
    GeometryBuffer::GetInstance().Resize(size);
}

void Game::SetScene(ScenePtr _scene, bool skipCreate)
{
    // if there is a current scene, call onQuit
    if (m_currentScene != nullptr)
    {
        m_currentScene->onQuit();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    // set the current scene to the new scene
    m_currentScene = std::move(_scene);
    if (!skipCreate)
    {
        m_currentScene->onCreate();
        m_currentScene->onCreateLate();
    }

    m_currentScene->onStart();
    m_currentScene->onLateStart();
}

ScenePtr Game::getScene()
{
    return m_currentScene;
}

void Game::onUpdate(float deltaTime)
{
    auto& inputManager = InputManager::GetInstance();
    auto& audioEngine = AudioEngine::GetInstance();
    inputManager.onUpdate();
    m_currentScene->onUpdate(deltaTime);
    audioEngine.Update();
}

void Game::onLateUpdate(float deltaTime)
{
    auto& inputManager = InputManager::GetInstance();
    m_currentScene->onLateUpdate(deltaTime);
    inputManager.onLateUpdate();
}

void Game::onFixedUpdate(float fixedDeltaTime)
{
    m_currentScene->onFixedUpdate(fixedDeltaTime);
}

void Game::onGraphicsUpdate()
{
    m_currentScene->onGraphicsUpdate();
}
