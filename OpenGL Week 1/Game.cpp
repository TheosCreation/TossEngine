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
#include <glew.h>
#include <glfw3.h>
#include "Window.h"
#include "VertexArrayObject.h"
#include "Shader.h"
#include "EntitySystem.h"
#include "GraphicsEntity.h"
#include "TextureCubeMap.h"
#include "Camera.h"
#include "SkyBoxEntity.h"
#include "Scene.h"
#include "Framebuffer.h"
#include "GeometryBuffer.h"
#include "SSRQuad.h"
#include "ProjectSettings.h"
#include "AudioEngine.h"

Game::Game()
{
    //init GLFW ver 4.6
    if (!glfwInit())
    {
        Debug::LogError("GLFW failed to initialize properly. Terminating program.");
        return;
    }
    m_projectSettings = std::make_unique<ProjectSettings>();
    m_projectSettings->LoadFromFile("ProjectSettings.json");

    initRandomSeed();
    m_display = std::make_unique<Window>(this);

    Vector2 windowSize = m_display->getInnerSize();
    GeometryBuffer::GetInstance().Init(windowSize);

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.Init(m_projectSettings);
    graphicsEngine.setViewport(windowSize);
    graphicsEngine.setDepthFunc(DepthType::Less);
    graphicsEngine.setBlendFunc(BlendType::SrcAlpha, BlendType::OneMinusSrcAlpha);
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
    graphicsEngine.setScissorSize(Rect(200, 200, 400, 300));
    graphicsEngine.setMultiSampling(true);

    auto& inputManager = InputManager::GetInstance();
    //inputManager.Init(m_projectSettings);
    inputManager.setGameWindow(m_display->getWindow());
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
    auto& resourceManager = ResourceManager::GetInstance();
    m_sphereMesh = resourceManager.createMeshFromFile("Resources/Meshes/sphere.obj");
    m_cubeMesh = resourceManager.createMeshFromFile("Resources/Meshes/cube.obj");

    auto& graphicsEngine = GraphicsEngine::GetInstance();

    auto scene = std::make_shared<Scene>(this);
    SetScene(scene);
}

void Game::onCreateLate()
{
    m_currentScene->onCreateLate();
}

void Game::onUpdateInternal()
{
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    auto& inputManager = InputManager::GetInstance();
    auto& audioEngine = AudioEngine::GetInstance();
    inputManager.onUpdate();

    // delta time
    m_currentTime = static_cast<float>(glfwGetTime());
    float deltaTime = m_currentTime - m_previousTime;
    m_previousTime = m_currentTime;

    // Accumulate time
    m_accumulatedTime += deltaTime;

    m_currentScene->onUpdate(deltaTime);
    audioEngine.Update();

    //if (inputManager.isKeyPressed(Key::Key1))
    //{
    //    auto scene1 = std::make_shared<Scene1>(this);
    //    SetScene(scene1);
    //}
    
    // Perform fixed updates
    while (m_accumulatedTime >= m_fixedTimeStep)
    {
        float fixedDeltaTime = m_currentTime - m_previousFixedUpdateTime;
        m_previousFixedUpdateTime = m_currentTime;
        m_currentScene->onFixedUpdate(fixedDeltaTime);
        m_accumulatedTime -= m_fixedTimeStep;
    }

    m_currentScene->onLateUpdate(deltaTime);
    inputManager.onLateUpdate();

    double RenderTime_Begin = (double)glfwGetTime();

    graphicsEngine.clear(glm::vec4(0, 0, 0, 1)); //clear the existing stuff first is a must
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    m_currentScene->onGraphicsUpdate(); //Render the scene

    ImGui::Begin("Settings menu lol");
    ImGui::Text("These are your options");

    static const char* items[]{ "Deferred Rendering","Forward" }; static int Selecteditem = (int)m_projectSettings->renderingPath;
    if (ImGui::Combo("Rendering Path", &Selecteditem, items, IM_ARRAYSIZE(items)))
    {
        RenderingPath selectedPath = static_cast<RenderingPath>(Selecteditem);
        Debug::Log("Rendering Path changed to option: " + ToString(selectedPath));
        graphicsEngine.setRenderingPath(selectedPath);
        m_projectSettings->renderingPath = selectedPath;
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    double RenderTime_End = (double)glfwGetTime();

    // Render to window
    m_display->present();
}

void Game::onQuit()
{
    m_currentScene->onQuit();
    quit();
}

void Game::run()
{
	onCreate();
	onCreateLate();

    //run funcs while window open
    while (m_display->shouldClose() == false)
    {
        glfwPollEvents();
        onUpdateInternal();
    }

    onQuit();
}

void Game::quit()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_display.release();

    m_projectSettings->SaveToFile("ProjectSettings.json");
}

void Game::onResize(int _width, int _height)
{
    GraphicsEngine::GetInstance().setViewport(Vector2(_width, _height));
    m_currentScene->onResize(_width, _height);
    GeometryBuffer::GetInstance().Resize(Vector2(_width, _height));
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

Window* Game::getWindow()
{
    return m_display.get();
}

float Game::GetCurrentTime()
{
    return m_currentTime;
}

MeshPtr Game::getCubeMesh()
{
    return m_cubeMesh;
}

MeshPtr Game::getSphereMesh()
{
    return m_sphereMesh;
}
//void Game::SetFullScreenShader(ShaderPtr _shader, Texture2DPtr _texture)
//{
//    if (_shader == nullptr)
//    {
//        m_canvasQuad->setShader(defaultQuadShader);
//    }
//    else
//    {
//        m_canvasQuad->setShader(_shader);
//    }
//    currentTexture1 = _texture;
//}